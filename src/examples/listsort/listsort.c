#include <mowgli.h>

int comparator(mowgli_node_t *n, mowgli_node_t *n2, void *opaque)
{
	int ret; 
	ret = strcasecmp(n->data, n2->data);

	return ret;
}

int main(int argc, char *argv[])
{
	mowgli_list_t l = {};
	mowgli_node_t *n;

	mowgli_init();

	mowgli_node_add("foo", mowgli_node_create(), &l);
	mowgli_node_add("bar", mowgli_node_create(), &l);
	mowgli_node_add("baz", mowgli_node_create(), &l);
	mowgli_node_add("splork", mowgli_node_create(), &l);
	mowgli_node_add("rabbit", mowgli_node_create(), &l);
	mowgli_node_add("meow", mowgli_node_create(), &l);
	mowgli_node_add("hi", mowgli_node_create(), &l);
	mowgli_node_add("konnichiwa", mowgli_node_create(), &l);
	mowgli_node_add("absolutely", mowgli_node_create(), &l);
	mowgli_node_add("cat", mowgli_node_create(), &l);
	mowgli_node_add("dog", mowgli_node_create(), &l);
	mowgli_node_add("woof", mowgli_node_create(), &l);
	mowgli_node_add("moon", mowgli_node_create(), &l);
	mowgli_node_add("new", mowgli_node_create(), &l);
	mowgli_node_add("delete", mowgli_node_create(), &l);
	mowgli_node_add("alias", mowgli_node_create(), &l);

	mowgli_list_sort(&l, comparator, NULL);

	MOWGLI_LIST_FOREACH(n, l.head)
	{
		printf("%s\n", n->data);
	}
}
