/* example_reg.c - main */

/********************************************************************************/
/*										*/
/* register a service with the inflection server				*/
/*										*/
/* use:   ./example_reg user pass service					*/
/*										*/
/* where user is a valid career account, pass is a random string that is	*/
/*	NOT the user's real password, service is service name			*/
/*										*/
/* The program sends a registration message to the inflection server, and then	*/
/* waits for an access app to connect and send a prompt.  When the prompt	*/
/* arrives, the program sends a reply that says					*/
/*										*/
/*		 "The reg app received the prompt: xxx"				*/
/*										*/
/* where xxx is a copy of the prompt that arrived.  After sending a reply, the	*/
/* reg app closes the TCP connection						*/
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
/* main -- main program for registration app to test the inflection server	*/
/*										*/
/********************************************************************************/


main (int argc, char *argv[]) {

	int		i;			/* loop index			*/
	int		n;			/* number of chars read		*/
	char		*user, *pass, *svc;	/* given by command line args	*/
	char		host[]="xinu00.cs.purdue.edu";	/* location of server	*/
	int		len;			/* string length temporaries	*/

	struct	cmd	*pcmd;			/* ptr to a registration command*/

	struct	hostent	*phe;			/* pointer to host info. entry	*/
	struct	protoent *ppe;			/* ptr. to protocol info. entry	*/
	struct	sockaddr_in socin;		/* an IPv4 endpoint address	*/
	int	addrlen;			/* len of a sockaddr_in struct	*/
	
	int	sock;				/* descriptor for socket	*/

	char	buffer[BUFF_SIZ];		/* input buffer for prompt	*/

	char	reply[] = "This is an echo from the reg side -> ";
						/* reply prefix			*/

	char	replybuf[BUFF_SIZ+sizeof(reply)];	/* reply buffer		*/

	/* check args */

	if (argc != 4) {
		fprintf(stderr, "use is:   ./example_reg user passwd service\n");
		exit(1);
	}

	user = argv[1];
	pass = argv[2];
	svc  = argv[3];

	if (strlen(user) > UID_SIZ) {
		fprintf(stderr, "user name %s is too long\n", user);
		exit(1);
	}

	if (strlen(pass) > PASS_SIZ) {
		fprintf(stderr, "password %s is too long\n", pass);
		exit(1);
	}

	if (strlen(svc) > SVC_SIZ) {
		fprintf(stderr, "Service name %s is too long\n", svc);
		exit(1);
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

	/* Form a registration command and send */

	pcmd = (struct cmd *) buffer;
	pcmd->cmdtype = CMD_REGISTER;

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

	len = strlen(svc);
	memset(pcmd->csvc, ' ', SVC_SIZ);
	memcpy(pcmd->csvc, svc, len);

	pcmd->dollar  = '$';

	/* Send registration message */

	send(sock, buffer, sizeof(struct cmd), 0);

	/* Wait for access app to respond by sending data */

	n = read(sock, buffer, BUFF_SIZ);

	if (n < 0) {
		fprintf(stderr, "error reading from the socket\n");
		exit(1);
	} else if (n == 0) {
		fprintf(stderr, "\nTCP connection was closed before a prompt arrived\n");
		exit(0);
	}

	/* prompt arrived from access app */

	buffer[n] = '\0';
	fprintf(stderr, "\nreg side -> received a prompt: %s\n", buffer);

	/* Send a reply */

	len = strlen(reply);
	memcpy(replybuf, reply, len);
	memcpy(&replybuf[len], buffer, n);
	send(sock, replybuf, len+n, 0);

	replybuf[len+n] = '\0';
	fprintf(stderr, "\nreg side -> sent a reply: %s\n", replybuf);

	fprintf(stderr, "\nreg side -> closing the TCP connection.\n");
	close(sock);
}
