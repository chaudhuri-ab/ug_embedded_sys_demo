/* example_acc.c - main */

/********************************************************************************/
/*										*/
/* Use the inflection server to access a previously-registered service		*/
/*										*/
/* use:   ./example_acc user pass service					*/
/*										*/
/* where user, pass, and service match the user ID, password, and service ID	*/
/* of a previously-registered service						*/
/*										*/
/* The program sends a registration message to the inflection server, and then	*/
/* sends a prompt to the reg app.  The program reads and displays the reply	*/
/* from the reg app until it encounters an end-of-file (i.e., the TCP connection*/
/* has been closed).								*/
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
#include <arpa/inet.h>

#include "inflection.h"
#include "utils.h"

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif /* INADDR_NONE */

/* Define buffer size */

#define  BUFF_SIZ 2048 /* size of a buffer for a connection	*/

/********************************************************************************/
/*										*/
/* main -- main program for access app to test the inflection server		*/
/*										*/

/********************************************************************************/


int main(int argc, char *argv[]) {

    int i; /* loop index			*/
    int n; /* number of chars read		*/
    char *user, *pass, *svc; /* given by command line args	*/
    char host[] = "xinu00.cs.purdue.edu"; /* location of server	*/
    char buffer[100]; /* I/O buffer			*/
    int len; /* string length temporaries	*/

    struct cmd *pcmd; /* ptr to a registration command*/

    struct hostent *phe; /* pointer to host info. entry	*/
    struct protoent *ppe; /* ptr. to protocol info. entry	*/
    struct sockaddr_in socin; /* an IPv4 endpoint address	*/

    int sock; /* descriptor for socket	*/


    /* check args */
    if (argc < 5) {
        fprintf(stderr, "use is:   ./example_acc user passwd service subserviceint(Get Temp = 1 | Get TIme = 2) [Opt Data]\n");
        exit(1);
    }

    user = argv[1];
    pass = argv[2];
    svc = argv[3];

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

    memset(&socin, 0, sizeof (socin));
    socin.sin_family = AF_INET;

    /* Map host name to IP address or map dotted decimal */

    if (phe = gethostbyname(host)) {
        memcpy(&socin.sin_addr, phe->h_addr, phe->h_length);
    } else if ((socin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE) {
        fprintf(stderr, "can't get host entry for %s\n", host);
        exit(1);
    }

    socin.sin_port = htons((unsigned short) TCPPORT);
    ppe = getprotobyname("tcp");

    /* Create the socket */

    sock = socket(PF_INET, SOCK_STREAM, ppe->p_proto);
    if (sock < 0) {
        fprintf(stderr, "cannot create socket\n");
        exit(1);
    }

    /* Connect the socket */

    if (connect(sock, (struct sockaddr *) &socin, sizeof (socin)) < 0) {
        fprintf(stderr, "can't connect to port %d\n", TCPPORT);
        exit(1);
    }

    /* Form an access command and send */

    pcmd = (struct cmd *) buffer;
    pcmd->cmdtype = CMD_ACCESS;

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

    pcmd->dollar = '$';

    /* Send registration message */

    send(sock, buffer, sizeof (struct cmd), 0);


    //Access Application Send Data Request
    int dataSize = 0;
    struct app_comm_msg req_msg;
    req_msg.header.action = atoi(argv[4]);
    req_msg.header.size = 0;

    if (argc == 6) {
        //copy any request data
        memcpy(req_msg.data, argv[5], strlen(argv[5]));
        req_msg.header.size += strlen(argv[5]);
    }
    printf("Sending: \n");
    print_app_comm_data(&req_msg);
    //put leading ints in network byte order
    dataSize = req_msg.header.size; //store size in host order
    req_msg.header.action = req_msg.header.action;
    req_msg.header.size = htons(req_msg.header.size);

    write(sock, &req_msg, dataSize + sizeof (struct app_comm_msg_header));

    fprintf(stderr, "\nacc side -> sent acc req\n");

    /* Read the reply from the service app */
    struct app_comm_msg reply_msg;
    n = read(sock, &(reply_msg.header), sizeof (reply_msg.header));

    if (n < 0) {
        fprintf(stderr, "error reading from the socket\n");
        exit(1);
    } else if (n == 0) {
        fprintf(stderr, "\nacc side -> TCP connection was closed before a reply arrived.\n");
        exit(0);
    } else if (n < sizeof (reply_msg.header)) {
        fprintf(stderr, "\nError Reading Header.\n");
        exit(0);
    }

    reply_msg.header.size = ntohs(reply_msg.header.size);
    reply_msg.header.action = reply_msg.header.action;

    if (reply_msg.header.size > 0) {
        //Read rest of message excluding size 
        n = read(sock, reply_msg.data, reply_msg.header.size);

        if (n < 0) {
            fprintf(stderr, "error reading from the socket\n");
            exit(1);
        } else if (n == 0) {
            fprintf(stderr, "\nacc side -> TCP connection was closed before a reply arrived.\n");
            exit(0);
        } else if (n < reply_msg.header.size) {
            fprintf(stderr, "\nDid not get full message.\n");
            exit(0);
        }

    }


    /* Data arrived -- print it */
    fprintf(stderr, "\nResponse:\n");
    print_app_comm_data(&reply_msg);





    //Clear slot + Close Connection

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

    pcmd->dollar = '$';

    /* Send connection clear message */

    send(sock, buffer, sizeof (struct cmd), 0);

    /* Read the reply from the server */

    n = read(sock, buffer, sizeof (buffer));

    while (n > 0) {
        buffer[n] = '\0';
        printf("\nServer says: %s\n", buffer);
        n = read(sock, buffer, sizeof (buffer));
    }
    if (n < 0) {
        fprintf(stderr, "error reading from the socket\n");
        exit(1);
    }
    printf("\nServer closed the connection\n");
    exit(0);
}







