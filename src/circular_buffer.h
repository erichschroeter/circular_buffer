#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#ifdef _WIN32
  #ifdef cb_EXPORTS
    #define CBAPI __declspec(dllexport)
  #else
    #define CBAPI __declspec(dllimport)
  #endif
  #define CBCALL __cdecl
#else
  #define CBAPI
  #define CBCALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32

#else /* UNIX */

#include <semaphore.h>

#endif

struct circular_buffer {
	char *buffer;
	int length;
	int tail;
	int head;
#ifdef WIN32
#else
	sem_t mutex;
#endif
};

CBAPI struct circular_buffer * CBCALL cb_create(int length);
CBAPI void CBCALL cb_destroy(struct circular_buffer *buffer);
CBAPI int CBCALL cb_read(struct circular_buffer *buffer, char *target, int amount);
CBAPI int CBCALL cb_read_single(struct circular_buffer *buffer, char *target);
CBAPI int CBCALL cb_write(struct circular_buffer *buffer, char *data, int length);
CBAPI int CBCALL cb_empty(struct circular_buffer *buffer);
CBAPI int CBCALL cb_full(struct circular_buffer *buffer);
CBAPI int CBCALL cb_available_data(struct circular_buffer *buffer);
CBAPI int CBCALL cb_available_space(struct circular_buffer *buffer);
CBAPI void CBCALL cb_debug(struct circular_buffer *buf);
CBAPI void CBCALL cb_clear(struct circular_buffer *buf);

#define cb_full(B) (cb_available_space((B)) == 0)
#define cb_empty(B) (cb_available_data((B)) == 0)
#define cb_starts_at(B) ((B)->buffer + (B)->tail)
#define cb_ends_at(B) ((B)->buffer + (B)->head)
#define cb_commit_read(B, A) ((B)->tail = ((B)->tail + (A)) % (B)->length)
#define cb_commit_write(B, A) ((B)->head = ((B)->head + (A)) % (B)->length)

#ifdef __cplusplus
}
#endif

#endif /* CIRCULAR_BUFFER_H */
