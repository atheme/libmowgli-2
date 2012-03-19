/* vio-udplistener.c - An example of the VIO API
 * To use: nc -u localhost, and then type stuff and hit enter. :p
 * This example is public domain.
 */

#include <mowgli.h>

#define BUFSIZE 2048

#define PROTO	AF_INET6
#define LISTEN	"::ffff:127.0.0.1"	/* 6to4 mapping */
#define PORT	31337

char *get_ip_str(const struct sockaddr *addr, char *str, size_t len)
{
	void *ptr;

	if (addr->sa_family == AF_INET)
		ptr = &((struct sockaddr_in *)addr)->sin_addr;
	else if (addr->sa_family == AF_INET6)
		ptr = &((struct sockaddr_in6 *)addr)->sin6_addr;
	else
		return NULL;

	if (inet_ntop(addr->sa_family, ptr, str, len) == NULL)
		return NULL;

	return str;
}

int main (void)
{
	mowgli_vio_t *vio = mowgli_vio_create(NULL);
	mowgli_vio_sockaddr_t *addr = mowgli_sockaddr_create(PROTO, LISTEN, 31337);

	if (mowgli_vio_socket(vio, PROTO, SOCK_DGRAM, 0))
		return EXIT_FAILURE;

	if (mowgli_vio_bind(vio, addr))
		return EXIT_FAILURE;

	while (true)
	{
		char buf[BUFSIZE] = "";
		char host[64];

		mowgli_vio_recvfrom(vio, buf, sizeof(buf), addr);

		get_ip_str((struct sockaddr *)&addr->addr, host, sizeof(host));

		printf("Recieved bytes from addr %s: %s", host, buf);

		mowgli_vio_sendto(vio, buf, strlen(buf), addr);
	}

	mowgli_free(addr);

	return EXIT_SUCCESS; /* Not reached */
}
