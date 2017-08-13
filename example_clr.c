/* example_clr.c - main */

/********************************************************************************/
/*										*/
/* Clear entries in the inflection server for a given user, password		*/
/*										*/
/* use:   ./example_clr user pass [service]					*/
/*										*/
/* where user and pass are the user ID and password for previously-registered	*/
/* entries, and service is an option name of a previously-registered service	*/
/* If no service is given, all service entries for the User and password are	*/
/* cleared.									*/
/*										*/
/* The program sends a connection close message to the inflection server.	*/
/*										*/
/********************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#include "inflection.h"

#ifndef	INADDR_NONE
#define	INADDR_NONE	0xffffffff
#endif	/* INADDR_NONE */

/* Define buffer size */

#define		BUFF_SIZ	2048	/* size of a buffer for a connection	*/

/********************************************************************************/
/*										*/
/* main -- main program for connection clear app to test the inflection server	*/
/*										*/
/********************************************************************************/

main (int argc, char *argv[]) {

	int		i;			/* loop index			*/
	int		n;			/* number of chars read		*/
	char		*user, *pass, *svc;	/* given by command line args	*/
	char		host[]="xinu00.cs.purdue.edu";	/* location of server	*/
	char		buffer[100];		/* I/O buffer			*/
	int		len;			/* string length temporaries	*/

	struct	cmd	*pcmd;			/* ptr to a registration command*/

	struct	hostent	*phe;			/* pointer to host info. entry	*/
	struct	protoent *ppe;			/* ptr. to protocol info. entry	*/
	struct	sockaddr_in socin;		/* an IPv4 endpoint address	*/
	int	addrlen;			/* len of a sockaddr_in struct	*/
	int	sock;				/* descriptor for socket	*/

	/* check args */

	if ( (argc != 3) && (argc != 4) ) {
		fprintf(stderr, "use is:   ./example_clr user passwd [service]\n");
		exit(1);
	}

	user = argv[1];
	pass = argv[2];

	if (strlen(user) > UID_SIZ) {
		fprintf(stderr, "user name %s is too long\n", user);
		exit(1);
	}

	if (strlen(pass) > PASS_SIZ) {
		fprintf(stderr, "password %s is too long\n", pass);
		exit(1);
	}

	if (argc == 4) {
		svc = argv[3];
		if (strlen(svc) > SVC_SIZ) {
			fprintf(stderr, "Service name %s is too long\n", svc);
			exit(1);
		}
	} else {
		svc = NULL;
	}

	/* Open socket used to connect to inflection server */

	memset(&socin, 0, sizeof(socin));
	socin.sin_family = AF_INET;

	/* Map host name to IP address or map dotted decimal */

	if ( phe = gethostbyname(host) ) {
		memcpy(&socin.sin_addr, phe->h_addr, phe->h_length);
	} else if ( (socin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE ) {
		fprintf(stderr, "can't get host entry for %s\n", host);
		exit(1);
	}

	socin.sin_port = htons( (unsigned short)TCPPORT );
	ppe = getprotobyname("tcp");

	/* Create the socket */

	sock = socket(PF_INET, SOCK_STREAM, ppe->p_proto);
	if (sock < 0) {
		fprintf(stderr, "cannot create socket\n");
		exit(1);
	}

	/* Connect the socket */

	if (connect(sock, (struct sockaddr *)&socin, sizeof(socin)) < 0) {
		fprintf(stderr, "can't connect to port %d\n", TCPPORT);
		exit(1);
	}

	/* Form an connection clear command and send */

	pcmd = (struct cmd *) buffer;
	pcmd->cmdtype = CMD_CLEAR;

	/* Add user ID */

	len = strlen(user);
	memset(pcmd->cid, ' ', UID_SIZ);
	memcpy(pcmd->cid, user, len);

	pcmd->cslash1 = '/';

	/* Add password */

	len = strlen(pass);
	memset(pcmd->cpass, ' ', PASS_SIZ);
	memcpy(pcmd->cpass, pass, len);

	pcmd->cslash2 = '/';

	/* Add service */

	memset(pcmd->csvc, ' ', SVC_SIZ);
	if (svc != NULL) {
		len = strlen(svc);
		memcpy(pcmd->csvc, svc, len);
	}

	pcmd->dollar  = '$';

	/* Send connection clear message */

	send(sock, buffer, sizeof(struct cmd), 0);

	/* Read the reply from the server */

	n = read(sock, buffer, sizeof(buffer));

	while (n > 0) {
		buffer[n] = '\0';
		printf("\nServer says: %s\n",buffer);
		n = read(sock, buffer, sizeof(buffer));
	}
	if (n < 0) {
		fprintf(stderr, "error reading from the socket\n");
		exit(1);
	}
	printf("\nServer closed the connection\n");
	exit(0);
}
