#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <semaphore.h>

struct circular_buffer {
	char *buffer;
	int length;
	int tail;
	int head;
	sem_t mutex;
};

struct circular_buffer *circular_buffer_create(int length);
void circular_buffer_destroy(struct circular_buffer *buffer);
int circular_buffer_read(struct circular_buffer *buffer, char *target, int amount);
int circular_buffer_write(struct circular_buffer *buffer, char *data, int length);
int circular_buffer_empty(struct circular_buffer *buffer);
int circular_buffer_full(struct circular_buffer *buffer);
int circular_buffer_available_data(struct circular_buffer *buffer);
int circular_buffer_available_space(struct circular_buffer *buffer);
void circular_buffer_debug(struct circular_buffer *buf);
void circular_buffer_clear(struct circular_buffer *buf);

#define circular_buffer_full(B) (circular_buffer_available_space((B)) == 0)
#define circular_buffer_empty(B) (circular_buffer_available_data((B)) == 0)
#define circular_buffer_starts_at(B) ((B)->buffer + (B)->tail)
#define circular_buffer_ends_at(B) ((B)->buffer + (B)->head)
#define circular_buffer_commit_read(B, A) ((B)->tail = ((B)->tail + (A)) % (B)->length)
#define circular_buffer_commit_write(B, A) ((B)->head = ((B)->head + (A)) % (B)->length)

#endif /* CIRCULAR_BUFFER_H */
