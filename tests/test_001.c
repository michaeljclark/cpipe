#undef NDEBUG
#include <stdio.h>
#include <assert.h>

#include "buffer.h"


void test_pbs()
{
	pbs_buffer pb;
	char buf1[384], buf2[384];

	for (int i = 0; i < sizeof(buf1); i++) buf1[i] = (char)i;

	pbs_buffer_init(&pb, 32768);

	for (int i = 0; i < 172; i++) {
		assert(pbs_buffer_write(&pb, buf1, sizeof(buf1)) == sizeof(buf1));
		memset(buf2, 0, sizeof(buf2));
		assert(pbs_buffer_read(&pb, buf2, sizeof(buf2)) == sizeof(buf2));
		assert(memcmp(buf1, buf2, sizeof(buf1)) == 0);
	}

	pbs_buffer_destroy(&pb);
}

void test_pbm()
{
	pbm_buffer pb;
	char buf1[384], buf2[384];

	for (int i = 0; i < sizeof(buf1); i++) buf1[i] = (char)i;

	pbm_buffer_init(&pb, 32768);

	for (int i = 0; i < 172; i++) {
		assert(pbm_buffer_write(&pb, buf1, sizeof(buf1)) == sizeof(buf1));
		memset(buf2, 0, sizeof(buf2));
		assert(pbm_buffer_read(&pb, buf2, sizeof(buf2)) == sizeof(buf2));
		assert(memcmp(buf1, buf2, sizeof(buf1)) == 0);
	}

	pbm_buffer_destroy(&pb);
}

int main(int argc, const char **argv)
{
	test_pbs();
	test_pbm();
}
