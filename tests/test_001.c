#undef NDEBUG
#include <stdio.h>
#include <assert.h>

#include "buffer.h"

void test_pb()
{
	pipe_buffer pb;
	char buf1[384], buf2[384];

	for (int i = 0; i < sizeof(buf1); i++) buf1[i] = (char)i;

	pipe_buffer_init(&pb, 32768);

	for (int i = 0; i < 172; i++) {
		assert(pipe_buffer_write(&pb, buf1, sizeof(buf1)) == sizeof(buf1));
		memset(buf2, 0, sizeof(buf2));
		assert(pipe_buffer_read(&pb, buf2, sizeof(buf2)) == sizeof(buf2));
		assert(memcmp(buf1, buf2, sizeof(buf1)) == 0);
	}

	pipe_buffer_destroy(&pb);
}

int main(int argc, const char **argv)
{
	test_pb();
}
