#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

struct circular_buffer {
	char *buffer;
	int length;
	int tail;
	int head;
	sem_t mutex;
};

struct circular_buffer *cb_create(int length);
void cb_destroy(struct circular_buffer *buffer);
int cb_read(struct circular_buffer *buffer, char *target, int amount);
int cb_read_single(struct circular_buffer *buffer, char *target);
int cb_write(struct circular_buffer *buffer, char *data, int length);
int cb_empty(struct circular_buffer *buffer);
int cb_full(struct circular_buffer *buffer);
int cb_available_data(struct circular_buffer *buffer);
int cb_available_space(struct circular_buffer *buffer);
void cb_debug(struct circular_buffer *buf);
void cb_clear(struct circular_buffer *buf);

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
