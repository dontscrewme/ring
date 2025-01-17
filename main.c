#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memcpy

struct ring_buffer {
  unsigned char *buffer;
  size_t head;
  size_t tail;
  size_t capacity;
  size_t element_size;
  bool full;
};

int ring_init(struct ring_buffer *ring, void *buffer, size_t capacity,
              size_t element_size) {
	if (!ring || !buffer || capacity == 0 || element_size == 0) {
			return -1;
	}

  ring->buffer = buffer;
  ring->capacity = capacity;
  ring->element_size = element_size;
  ring->head = 0;
  ring->tail = 0;
  ring->full = false;

  return 0;
}

int ring_push(struct ring_buffer *ring, void *data) {
  if (ring->full) {
    return -1;
  }

  unsigned char *dest = ring->buffer + (ring->head * ring->element_size);
  memcpy(dest, data, ring->element_size);

  ring->head++;
  if (ring->head == ring->capacity) {
    ring->head = 0;
  }

  if (ring->head == ring->tail) {
    ring->full = true;
  }

  return 0;
}

int ring_pop(struct ring_buffer* ring, void* data)
{
	if (ring->head == ring->tail && (ring->full == false))
	{
		return -1;
	}

	unsigned char* source = ring->buffer + (ring->tail * ring->element_size);
	memcpy(data, source, ring->element_size);

	ring->tail++;
	if (ring->tail == ring->capacity)
	{
		ring->tail = 0;
	}

	ring->full = false;


	return 0;
}

int get_ring_status(struct ring_buffer* ring, bool* status)
{
	if (!ring)
	{
		return -1;
	}

	*status = ring->full;

	return 0;
}

int main() {
	int ring[32] = {0};
	struct ring_buffer int_ring;
	ring_init(&int_ring, ring, 32, sizeof(int));

	int value = -1;
	if (ring_push(&int_ring, &value) == 0) {
			printf("Pushed: %d\n", value);
	} else {
			printf("Failed to push: %d\n", value);
	}

	int ret;
	if (ring_pop(&int_ring, &ret) == 0) {
			printf("Popped: %d\n", ret);
	} else {
			printf("Failed to pop\n");
	}

	struct myStruct {
			int id;
			char name[16];
	} temp[32];

	struct ring_buffer struct_ring;
	ring_init(&struct_ring, temp, 32, sizeof(struct myStruct));

	struct myStruct new_item = {1, "Test"};
	if (ring_push(&struct_ring, &new_item) == 0) {
			printf("Pushed: id=%d, name=%s\n", new_item.id, new_item.name);
	} else {
			printf("Failed to push struct\n");
	}

	struct myStruct popped_item;
	if (ring_pop(&struct_ring, &popped_item) == 0) {
			printf("Popped: id=%d, name=%s\n", popped_item.id, popped_item.name);
	} else {
			printf("Failed to pop struct\n");
	}

	return 0;
}
