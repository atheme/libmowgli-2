/* vio-udplistener.c - An example of the VIO API
 * To use: nc -u localhost, and then type stuff and hit enter. :p
 * This example is public domain.
 */

#include <mowgli.h>

#define BUFSIZE 2048

int main (void)
{
	mowgli_vio_t *vio = mowgli_vio_create(NULL);
	mowgli_vio_sockaddr_t *addr = mowgli_sockaddr_create(AF_INET, NULL, 31337);

	if (mowgli_vio_socket(vio, AF_INET, SOCK_DGRAM, 0))
		return EXIT_FAILURE;

	if (mowgli_vio_bind(vio, addr))
		return EXIT_FAILURE;

	while (true)
	{
		char buf[BUFSIZE] = "";
		char host[64];

		mowgli_vio_recvfrom(vio, buf, sizeof(buf), addr);

		inet_ntop(AF_INET, &((struct sockaddr_in*)&addr->addr)->sin_addr, host, sizeof(host));

		printf("Recieved bytes from addr %s: %s", host, buf);

		mowgli_vio_sendto(vio, buf, strlen(buf), addr);
	}

	mowgli_free(addr);

	return EXIT_SUCCESS; /* Not reached */
}
