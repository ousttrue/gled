#include <msgpack.h>

/* msgpack notifications */
static gled_notification notifications[MAX_NOTIFICATIONS];
static int num_notifications;

/* msgpack responses */
static gled_response responses[MAX_RESPONSES];
static int num_responses;

/* TODO: */
/* static gled_vim_str2funcID function_map; */

static uint64_t gled_method_id(const char *func);


void get_response(msgpack_object_array *msg)
{
  /* response format:
   *  (id, nil, ret) or
   *  (id, error, nil) */
  gled_response reply;

  reply = &responses[num_responses];
  reply->id = msg[0]->ptr->via.u64;
  if(!msg[1]->ptr->type == MSGPACK_OBJECT_NIL){
    reply->val = msg[1]->ptr->via.u64;
  }else{
    reply->val = msg[2]->ptr->via.u64;
  }
  num_responses++;
}

void get_notification(msgpack_object_array *msg)
{
  /* notification format:
   * (name, args) */
  gled_notificatio note;

  note = &notifications[num_notifications];
  note->name[sizeof(note->name)-1] = '\0';
  strncpy(note->name, msg[0]->ptr->via.str, sizeof(note->name)-1);
  note->args = *msg[1]->ptr;

  num_notifications++;
}

void gled_handle_messages()
{
  msgpack_unpacker pac;
  msgpack_object result;
  msgpack_sbuffer *buffer;

  buffer = msgpack_sbuffer_new();
  msgpack_unpacker_reserv_buffer(&pac, buffer->size);
  memcpy(msgpack_unpacker_buffer(&pac), buffer->data, buffer->size);
  msgpack_unpacker_buffer_consumed(&pac, buffer->size);

  msgpack_unpacked_init(&msg);
  while(msgpack_unpacker_next(&pac, &result)){
    msgpack_object_array *arr = &result.via.array;
    if(strncmp(object.via.str, "RESPONSE", 8) == 0 && len == 4){
      get_response(arr);
    }else if(strncmp(object.via.str, "NOTIFY", 6) == 0 && len == 3){
      get_notification(arr);
    }else{
      fprintf(stderr, "Unknown message type %u\n", msg.via.u64);
    }
  }
  msgpack_sbuffer_destroy(buffer);
}

gled_notification * gled_server_poll_notification()
{
  if(num_notifications == 0){
    return;
  }
  num_notifications--;
  return &notifications[num_notifications--];
}

gled_response * gled_server_poll_responses()
{
  if(num_responses == 0){
    return;
  }
  num_responses--;
  return &responses[num_responses--];
}

uint64_t gled_method_id(const char *func)
{
  /* TODO: get the ID of the function from the string->ID table (uthash) */
}

void gled_current(const char *prop)
{
  char request[128] = "vim_get_current_";
  strncat(request, prop, 100);
}

uint64_t gled_request(uint64_t id, msgpack_object *args)
{
  /* TODO: send the request */
}