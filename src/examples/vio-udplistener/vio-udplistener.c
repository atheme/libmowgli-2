/* vio-udplistener.c - An example of the VIO API
 * To use: nc -u localhost, and then type stuff and hit enter. :p
 * This example is public domain.
 */

#include <mowgli.h>

#define BUFSIZE 2048

#define PROTO AF_INET6
#define LISTEN "::ffff:127.0.0.1"	/* 6to4 mapping */
#define PORT 31337

#define ECHOBACK "Echo: "

int
main(void)
{
	mowgli_vio_t *vio = mowgli_vio_create(NULL);
	mowgli_vio_sockaddr_t addr;

	mowgli_vio_sockaddr_create(&addr, PROTO, LISTEN, PORT);

	if (mowgli_vio_socket(vio, PROTO, SOCK_DGRAM, 0))
		return EXIT_FAILURE;

	if (mowgli_vio_bind(vio, &addr))
		return EXIT_FAILURE;

	while (true)
	{
		char buf[BUFSIZE] = "";
		mowgli_vio_sockdata_t sockinfo;

		mowgli_vio_recvfrom(vio, buf, sizeof(buf), &addr);

		mowgli_vio_sockaddr_info(&addr, &sockinfo);

		printf("Recieved bytes from addr [%s]:%hu: %s", sockinfo.host, sockinfo.port, buf);

		mowgli_vio_sendto(vio, ECHOBACK, sizeof(ECHOBACK), &addr);
		mowgli_vio_sendto(vio, buf, strlen(buf), &addr);
	}

	return EXIT_SUCCESS;	/* Not reached */
}
