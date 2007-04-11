#include <mowgli.h>

int main(int argc, char *argv[])
{
	mowgli_random_t *r = mowgli_random_new();
	int i;

	printf("1000 iterations:\n");
	for (i = 0; i < 1000; i++)
	{
		printf("%10u ", mowgli_random_int(r));
		if (i % 5 == 4) printf("\n");
	}

	mowgli_random_destroy(r);

	return 0;
}
