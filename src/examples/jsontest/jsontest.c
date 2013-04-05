#include <mowgli.h>

void
out_string(mowgli_json_output_t *out, const char *str, size_t len)
{
	fwrite(str, 1, len, stdout);
}

void
out_char(mowgli_json_output_t *out, const char c)
{
	fputc(c, stdout);
}

mowgli_json_output_t out =
{
	.append = out_string,
	.append_char = out_char,
};

int
main(int argc, char *argv[])
{
	int i;
	mowgli_json_t *n;

	if (argc < 2)
	{
		printf("Usage: %s file [file ...]\n", argv[0]);
		return 1;
	}

	for (i = 1; i < argc; i++)
	{
		n = mowgli_json_parse_file(argv[i]);

		if (n != NULL)
		{
			mowgli_json_serialize(n, &out, 1);
			putchar('\n');
		}

		mowgli_json_decref(n);
	}

	return 0;
}
