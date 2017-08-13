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
#include <time.h>


#include "inflection.h"

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif /* INADDR_NONE */

/* Define buffer size */

#define  BUFF_SIZ 2048 /* size of a buffer for a connection	*/

//Application Prototypes

void clk(char*, size_t len);
void temp(char*, size_t len);

//


/********************************************************************************/
/*										*/
/* main -- main program for registration app to test the inflection server	*/
/*										*/

/********************************************************************************/


main(int argc, char *argv[]) {

#ifdef SYS_LIL_ENDIAN 
    printf("lil endian\n");
#endif

    int i; /* loop index			*/
    int n; /* number of chars read		*/
    char *user, *pass, *svc; /* given by command line args	*/
    char host[] = "xinu00.cs.purdue.edu"; /* location of server	*/
    int len; /* string length temporaries	*/

    struct cmd *pcmd; /* ptr to a registration command*/

    struct hostent *phe; /* pointer to host info. entry	*/
    struct protoent *ppe; /* ptr. to protocol info. entry	*/
    struct sockaddr_in socin; /* an IPv4 endpoint address	*/
    int addrlen; /* len of a sockaddr_in struct	*/

    int sock; /* descriptor for socket	*/

    char buffer[BUFF_SIZ]; /* input buffer for prompt	*/

    char reply[] = "This is an echo from the reg side -> ";
    /* reply prefix			*/

    char replybuf[BUFF_SIZ + sizeof (reply)]; /* reply buffer		*/

    /* check args */

    if (argc != 4) {
        fprintf(stderr, "use is:   ./example_reg user passwd service\n");
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

    while (1) {
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

        pcmd->dollar = '$';

        /* Send registration message */

        send(sock, buffer, sizeof (struct cmd), 0);

        /* Wait for access app to respond by sending data */

        //Embedded Dev Action Switch

        struct app_comm_msg req_msg;
        n = read(sock, &(req_msg.header), sizeof (struct app_comm_msg_header));

        if (n < 0) {
            fprintf(stderr, "error reading from the socket\n");
            continue;
        } else if (n == 0) {
            fprintf(stderr, "\nacc side -> TCP connection was closed before a reply arrived.\n");
            continue;
        } else if (n < sizeof (req_msg.header)) {
            fprintf(stderr, "\nError Reading Header.\n");
            continue;
        }

        //Convert to host byte order
        req_msg.header.size = ntohl(req_msg.header.size);
        req_msg.header.action = ntohl(req_msg.header.action);

        //Get Data

        if (req_msg.header.size > 0) {
            //Read rest of message excluding size 
            n = read(sock, req_msg.data, req_msg.header.size);

            if (n < 0) {
                fprintf(stderr, "error reading from the socket\n");
                continue;
            } else if (n == 0) {
                fprintf(stderr, "\nacc side -> TCP connection was closed before a reply arrived.\n");
                continue;
            } else if (n < req_msg.header.size) {
                fprintf(stderr, "\nDid not get full message.\n");
                continue;
            }

        }

        //


        //Action Switch Controller
        struct app_comm_msg reply_msg;

        switch (req_msg.header.action) {
            case GET_TEMP:
            {
                printf("Get Temp Req\n");
                print_app_comm_data(&req_msg);

                reply_msg.header.action = GET_TEMP;
                reply_msg.header.size = 100;
                clk(reply_msg.data, 100);


            }
                break;

            case GET_TIME:
            {
                printf("Get Time Req\n");
                print_app_comm_data(&req_msg);

                reply_msg.header.action = GET_TIME;
                reply_msg.header.size = 100;
                temp(reply_msg.data, 100);


            }
                break;

            default:
                continue;


        }


        printf("Sending: \n");
        print_app_comm_data(&reply_msg);
        //put leading ints in network byte order
        int dataSize = reply_msg.header.size; //store size in host order
        reply_msg.header.action = htonl(reply_msg.header.action);
        reply_msg.header.size = htonl(reply_msg.header.size);

        write(sock, &reply_msg, dataSize + sizeof (struct app_comm_msg_header));
        close(sock);
    }
}

/**
 * Writes formatted string to buffer
 * 
 * 
 * @param buffer - string buffer to write to 
 * @param len length of buffer
 */
void clk(char* buffer, size_t len) {
    memset(buffer, 0, len);
    time_t timer;
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, len, "%Y-%m-%d %H:%M:%S", tm_info);
}

/**
 * Simulated temp reading 
 * 
 * 
 * @param buffer - string buffer to write to
 * @param len - length of buffer
 */
void temp(char* buffer, size_t len) {
    //simulation
    memset(buffer, 0, len);
    static int temp = 100;
    snprintf(buffer, len, "%d F", temp++);
}