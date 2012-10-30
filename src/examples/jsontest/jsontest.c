#include <mowgli.h>

mowgli_string_t *serialized;

void dump_json(mowgli_json_t *n)
{
	mowgli_string_reset(serialized);

	mowgli_json_serialize(n, serialized, 1);

	printf("%s\n", serialized->str);
}

int main(int argc, char *argv[])
{
	int i;
	mowgli_json_t *n;

	if (argc < 2)
	{
		printf("Usage: %s file [file ...]\n", argv[0]);
		return 1;
	}

	serialized = mowgli_string_create();

	for (i=1; i<argc; i++)
	{
		n = mowgli_json_parse_file(argv[i]);

		if (n != NULL)
			dump_json(n);

		mowgli_json_decref(n);
	}

	mowgli_string_destroy(serialized);

	return 0;
}
