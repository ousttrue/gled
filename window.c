#include "window.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include "matrix.h"
#include "rune.h"
#include "util.h"
#include "vector.h"

Window *new_Window(uint32_t width, uint32_t height) {
	uint32_t i, j;
	Window *w;

	w = malloc(sizeof(Window));
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			    SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	w->win = SDL_CreateWindow(
	    "gled", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width * 32,
	    height * 32, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (w->win == NULL) {
		puts("error: failed to create window.");
		return NULL;
	}
	w->ctx = SDL_GL_CreateContext(w->win);
	if (w->ctx == NULL) {
		puts("error: failed to create GL context");
		return NULL;
	}
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		puts("error: failed to initialize GLEW");
		return NULL;
	}
	w->w = width;
	w->h = height;

	for (i = 0; i < height; ++i) {
		for (j = 0; j < width; ++j) {
			w->buff[i][j].ch = rune_blankChar;
		}
	}

	/* TODO: test */
	w->buff[2][3].img = rune_blankImg;
	w->buff[2][3].img.filename = "fonts/ascii.bmp";

	MeshRune m = rune_blankMesh;
	m.filename = "cube.obj";
	m.r.w = 3;
	m.r.h = 3;
	window_setMesh(w, 0, 0, &m);

	return w;
}

void del_Window(Window *w) {
	if (w == NULL) {
		return;
	}
	if (w->ctx != NULL) {
		SDL_GL_DeleteContext(w->ctx);
	}
	if (w->win) {
		SDL_DestroyWindow(w->win);
	}
	free(w);
}

/* window_DrawRune applies the target render in tex from the rect (u,v,w,h) */
void window_DrawRune(GLuint tex, Rect pos, Rect clip) {
	const GLchar *vs =
	    "#version 150\n"
	    "in vec2 pos;\n"
	    "in vec2 texco;\n"
	    "out vec2 out_texco;\n"
	    "uniform sampler2D tex;\n"
	    "uniform mat4 mvp;\n"
	    "void main()\n"
	    "{\n"
	    "  out_texco = texco;\n"
	    "  gl_Position = mvp * vec4(pos, 0.0, 1.0);\n"
	    "}\n";
	const GLchar *fs =
	    "#version 150\n"
	    "in vec2 out_texco;\n"
	    "out vec4 out_color;\n"
	    "uniform sampler2D tex;\n"
	    "void main()\n"
	    "{\n"
	    "  out_color = texture(tex, out_texco);\n"
	    "}\n";
	static GLuint vao = 0;
	static GLuint vbo = 0;
	static GLuint ibo = 0;
	static GLfloat vertices[4 * 4] = {
	    /* 4 vertices. format: x-y-u-v */
	    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	    1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	};
	static GLshort indices[3 * 2] = {/* 2 triangles */
					 0, 1, 2, 0, 3, 2};
	static Mat4x4 mvp;
	static GLuint mvpUniform;
	static GLuint texUniform;
	static GLuint shader;

	/* create shader program */
	if (shader == 0) {
		const char *attrs[31] = {"pos", "texco"};
		shader = loadShader(vs, fs, 2, attrs);

		/* get the uniforms */
		mvpUniform = glGetUniformLocation(shader, "mvp");
		mat4x4_orthographic(&mvp, 0.0f, 80.0f, 0.0f, 40.0f, -1.0f,
				    1.0f);
		texUniform = glGetUniformLocation(shader, "tex");
	}

	/* create vertex attribute object */
	if (vao == 0) {
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ibo);

		/* vertices */
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 4, vertices,
			     GL_STATIC_DRAW);

		/* indices */
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLshort) * 6,
			     indices, GL_STATIC_DRAW);
	}

	if (tex == 0) {
		return;
	}

	/* set VBO attributes */
	vertices[0] = pos.x;
	vertices[1] = pos.y;
	vertices[2] = clip.x;
	vertices[3] = clip.y;

	vertices[0 + 4] = pos.x + pos.w;
	vertices[1 + 4] = pos.y;
	vertices[2 + 4] = clip.x + clip.w;
	vertices[3 + 4] = clip.y;

	vertices[0 + 8] = pos.x + pos.w;
	vertices[1 + 8] = pos.y + pos.h;
	vertices[2 + 8] = clip.x + clip.w;
	vertices[3 + 8] = clip.y + clip.h;

	vertices[0 + 12] = pos.x;
	vertices[1 + 12] = pos.y + pos.h;
	vertices[2 + 12] = clip.x;
	vertices[3 + 12] = clip.y + clip.h;

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 4, vertices,
		     GL_STATIC_DRAW);

	/* draw */
	glUseProgram(shader);
	glUniformMatrix4fv(mvpUniform, 1, 0, ((GLfloat *)&mvp));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(texUniform, 0);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4,
			      (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4,
			      (void *)8);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *)0);
}

/* window_redraw renders the window by drawing all runes in the render area. */
void window_redraw(Window *w) {
	unsigned int i, j, k, l;

	/* mark all runes as 'dirty' so that they will be drawn */
	for (i = 0; i < w->h; ++i) {
		for (j = 0; j < w->w; ++j) {
			w->buff[i][j].r.flags.dirty = true;
		}
	}

	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < w->h; ++i) {
		for (j = 0; j < w->w; ++j) {
			/* draw the rune @ (j, i) if it is dirty */
			Rune *r = &w->buff[i][j].r;
			if (r->flags.dirty && r->draw != NULL) {
				RuneDrawResult res;
				res = r->draw(r, j, i);
				res.pos.x += i;
				res.pos.y += j;
				window_DrawRune(res.tex, res.pos, res.clip);
			}
			/* mark the area that this rune renders to as 'clean' */
			for (k = 0; (k < r->h) && (k < w->h); ++k) {
				for (l = 0; (l < r->w) && (l < w->w); ++l) {
					Rune *clean;
					clean = &w->buff[i + k][j + l].r;
					clean->flags.dirty = false;
				}
			}
		}
	}
	SDL_GL_SwapWindow(w->win);
}

/* window_update updates all runes within the window's render area. */
void window_update(Window *w) {
	unsigned int i, j, k, l;

	/* mark all runes as 'dirty' so that they will be updated */
	for (i = 0; i < w->h; ++i) {
		for (j = 0; j < w->w; ++j) {
			w->buff[i][j].r.flags.dirty = true;
		}
	}

	for (i = 0; i < w->h; ++i) {
		for (j = 0; j < w->w; ++j) {
			/* update the rune @ (j, i) if it is dirty */
			Rune *r;
			r = &(w->buff[i][j].r);
			if (r->flags.dirty && r->update != NULL) {
				r->update(r);
			}
			/* mark the area that this rune updated as 'clean' */
			for (k = i; k < (r->h + w->h) && (k < w->h); ++k) {
				for (l = j; l < (r->w + w->w) && (l < w->w);
				     ++l) {
					r->flags.dirty = false;
				}
			}
		}
	}
}

/* window_resize resizes win to cols x rows tiles */
void window_resize(Window *win, uint32_t cols, uint32_t rows) {
	win->w = cols;
	win->h = rows;
}

/* window_at returns a reference to the rune at (x, y). */
Rune_ *window_at(Window *win, uint32_t x, uint32_t y) {
	return &win->buff[x][y];
}

void window_setChar(Window *w, uint32_t x, uint32_t y, CharRune *r) {
	memcpy(&(window_at(w, x, y)->ch), r, sizeof(CharRune));
}

void window_setMesh(Window *w, uint32_t x, uint32_t y, MeshRune *r) {
	memcpy(&(window_at(w, x, y)->mesh), r, sizeof(MeshRune));
}

void window_setImg(Window *w, uint32_t x, uint32_t y, ImgRune *r) {
	memcpy(&(window_at(w, x, y)->img), r, sizeof(ImgRune));
}
