#ifndef UTIL_H
#define UTIL_H

#include <GL/glew.h>
#include <SDL2/SDL.h>

#define GL_CHECK()                                                             \
	{                                                                      \
		GLenum err;                                                    \
		if ((err = glGetError()) != GL_NO_ERROR) {                     \
			printf("gl error [%s line: %d] (%d: %s)\n", *__FILE__, \
			       __LINE__, err, gluErrorString(err));            \
		}                                                              \
	}

#endif

GLuint loadShader(const GLchar *, const GLchar *, int, const char **);
