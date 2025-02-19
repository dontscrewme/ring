#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memcpy
#include <assert.h>

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

/* ============= TEST PROGRAM ============= */

/* A small struct to demonstrate storing custom data types. */
typedef struct {
    int x;
    float y;
} custom_t;

int main(void)
{
    printf("=== Testing ring buffer with INT ===\n");
    {
        struct ring_buffer ring_int;
        /* Enough space for 5 integers */
        unsigned char buffer_int[5 * sizeof(int)];

        int ret = ring_init(&ring_int, buffer_int, 5, sizeof(int));
        assert(ret == 0);

        /* Push and pop a few integers */
        int data_in, data_out;

        /* Fill with 0, 10, 20, 30, 40 */
        for (int i = 0; i < 5; i++) {
            data_in = i * 10;
            ret = ring_push(&ring_int, &data_in);
            assert(ret == 0);
        }

        /* Confirm ring is full */
        bool is_full = false;
        ret = get_ring_status(&ring_int, &is_full);
        assert(ret == 0);
        assert(is_full == true);

        /* Pop all and verify correctness (0,10,20,30,40) */
        for (int i = 0; i < 5; i++) {
            ret = ring_pop(&ring_int, &data_out);
            assert(ret == 0);
            assert(data_out == i * 10);
        }
        /* Confirm ring is empty after popping all */
        ret = ring_pop(&ring_int, &data_out);
        assert(ret == -1);

        printf("INT test passed.\n");
    }

    printf("=== Testing ring buffer with FLOAT ===\n");
    {
        struct ring_buffer ring_float;
        /* Enough space for 5 floats */
        unsigned char buffer_float[5 * sizeof(float)];

        int ret = ring_init(&ring_float, buffer_float, 5, sizeof(float));
        assert(ret == 0);

        /* Push and pop floats */
        float data_in, data_out;

        /* Fill with: 0.0, 1.1, 2.2, 3.3, 4.4 */
        for (int i = 0; i < 5; i++) {
            data_in = i * 1.1f;  
            ret = ring_push(&ring_float, &data_in);
            assert(ret == 0);
        }

        /* Confirm ring is full */
        bool is_full = false;
        ret = get_ring_status(&ring_float, &is_full);
        assert(ret == 0);
        assert(is_full == true);

        /* Pop all, verify correctness */
        for (int i = 0; i < 5; i++) {
            ret = ring_pop(&ring_float, &data_out);
            assert(ret == 0);
            float expected = i * 1.1f;
            /* Check floating values with small epsilon */
            assert((data_out - expected) < 1e-6 && (data_out - expected) > -1e-6);
        }
        /* Confirm ring is empty after popping all */
        ret = ring_pop(&ring_float, &data_out);
        assert(ret == -1);

        printf("FLOAT test passed.\n");
    }

    printf("=== Testing ring buffer with CUSTOM STRUCT ===\n");
    {
        struct ring_buffer ring_struct;
        /* Enough space for 5 custom_t elements */
        unsigned char buffer_struct[5 * sizeof(custom_t)];

        int ret = ring_init(&ring_struct, buffer_struct, 5, sizeof(custom_t));
        assert(ret == 0);

        custom_t data_in, data_out;

        /* Insert 5 custom_t objects, e.g. (0,0.0), (1,1.5), (2,3.0)... */
        for (int i = 0; i < 5; i++) {
            data_in.x = i;
            data_in.y = i * 1.5f;
            ret = ring_push(&ring_struct, &data_in);
            assert(ret == 0);
        }

        /* Confirm ring is full */
        bool is_full = false;
        ret = get_ring_status(&ring_struct, &is_full);
        assert(ret == 0);
        assert(is_full == true);

        /* Pop all and check them */
        for (int i = 0; i < 5; i++) {
            ret = ring_pop(&ring_struct, &data_out);
            assert(ret == 0);
            float expected_y = i * 1.5f;
            assert(data_out.x == i);
            assert((data_out.y - expected_y) < 1e-6 && (data_out.y - expected_y) > -1e-6);
        }

        /* Confirm ring is empty after popping all */
        ret = ring_pop(&ring_struct, &data_out);
        assert(ret == -1);

        printf("CUSTOM STRUCT test passed.\n");
    }

    /*
      === Testing scenario where tail is ahead of head ===
      We won't look at ring->tail or ring->head directly.
      Instead, we'll push/pop in a way that the ring wraps
      around so tail ends up "ahead" of head in circular indexing.
    */
    printf("=== Testing scenario where tail is ahead of head ===\n");
    {
        struct ring_buffer ring_wrap;
        /* Enough space for 5 integers */
        unsigned char buffer_wrap[5 * sizeof(int)];

        int ret = ring_init(&ring_wrap, buffer_wrap, 5, sizeof(int));
        assert(ret == 0);

        int data_in, data_out;

        /* Step 1: Push 3 integers: 0, 1, 2 */
        for (int i = 0; i < 3; i++) {
            data_in = i;
            ret = ring_push(&ring_wrap, &data_in);
            assert(ret == 0);
        }
        /* Step 2: Pop 2 integers => pops 0, then 1 */
        for (int i = 0; i < 2; i++) {
            ret = ring_pop(&ring_wrap, &data_out);
            assert(ret == 0);
            assert(data_out == i); /* 0, then 1 */
        }
        /* Step 3: Push 2 more => let's say 99, 100
           Because we popped two, we haven't yet wrapped around fully,
           but soon we'll cause the ring to wrap so that the 'head'
           goes back to 0 while 'tail' is still > 0. */
        data_in = 99;
        ret = ring_push(&ring_wrap, &data_in);
        assert(ret == 0);
        data_in = 100;
        ret = ring_push(&ring_wrap, &data_in);
        assert(ret == 0);

        /* At this point, the ring has items: [2, 99, 100].
           The tail is "ahead" of the head in circular terms.
           Now let's pop the remaining 3 items in FIFO order:
             2, 99, 100
        */
        int expected[3] = {2, 99, 100};
        for (int i = 0; i < 3; i++) {
            ret = ring_pop(&ring_wrap, &data_out);
            assert(ret == 0);
            assert(data_out == expected[i]);
        }

        /* Ring is empty now; next pop should fail */
        ret = ring_pop(&ring_wrap, &data_out);
        assert(ret == -1);

        printf("Wrap-around test (tail ahead of head) passed.\n");
    }

    printf("All ring buffer tests with multiple data types (and wrap-around) passed!\n");
    return 0;
}
