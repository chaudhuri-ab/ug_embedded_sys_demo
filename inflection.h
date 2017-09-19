#ifndef INFLECTION_H
#define INFLECTION_H
/* Common definintions for client and server sides for inflection server	*/

/* Define sizes */

#define  UID_SIZ  10 /* Bytes in a user ID			*/
#define  PASS_SIZ 8 /* Bytes in a password			*/
#define  SVC_SIZ  10 /* Bytes in a service name. Change next	*/
/*   line if size changes		*/
#define  SVC_BLANK "          " /* SVC_SIZ blank characters	*/

/* TCP port to use */

#define  TCPPORT  49909 /* TCP port the service will use	*/

/* structure of a command */

struct cmd { /* format of a command			*/
    char cmdtype; /* command type (see CMD_* above)	*/
    char cid[UID_SIZ]; /* user ID (blank-padded on right)	*/
    char cslash1; /* Must be a slash character 		*/
    char cpass[PASS_SIZ]; /* password (blank padded on right)	*/
    char cslash2; /* Must be a slash character		*/
    char csvc[SVC_SIZ]; /* service name				*/
    char dollar; /* Must be a dollar sign		*/
};

#define  CMD_SIZ  sizeof(struct cmd) /* bytes in a command	*/

/* Definition of command types */

#define  CMD_ACCESS 'A' /* Command type for "Access"		*/
#define  CMD_CLEAR 'C' /* Command type for "Clear"		*/
#define  CMD_REGISTER 'R' /* Command type for "Register"		*/


// App Data Frame
#pragma pack(push, 1) // exact fit - no padding

struct app_comm_msg_header {
    unsigned short size; /*size of comm data excluding header*/
    unsigned char action;
};

struct app_comm_msg {
    struct app_comm_msg_header header;
    char data[1400];
};

#pragma pack(pop) //back to whatever the previous packing mode was 

//

//App Action Requests
#define GET_TEMP  1
#define GET_TIME  2

//



/* NETWORK BYTE ORDER CONVERSION NOT NEEDED ON A BIG-ENDIAN MACHINES */

/*
    Notes:
        - Intel CPUs are little endian, ARMs are configurable
        - Network Byte Order Is Big Endian
        - http://www.geeksforgeeks.org/little-and-big-endian-mystery/
        - https://serverfault.com/questions/163487/how-to-tell-if-a-linux-system-is-big-endian-or-little-endian
        - htons where s is for short and htonl is for integers that are 32 bits. 
            h stands for host (i.e. the machine sending or receiving packets). n stands for network.
            htons is read host to network short. ntohs is read network to host short.  
 */

#define htons(x)  ((0xff & ((x)>>8)) | ((0xff & (x)) << 8))
#define htonl(x)  ((((x)>>24) & 0x000000ff) | (((x)>> 8) & 0x0000ff00) | \
                   (((x)<<8) & 0x00ff0000) | (((x)<<24) & 0xff000000))
#define ntohs(x)  ((0xff & ((x)>>8)) | ( (0xff & (x)) << 8))
#define ntohl(x)  ((((x)>>24) & 0x000000ff) | (((x)>> 8) & 0x0000ff00) | \
                   (((x)<<8) & 0x00ff0000) | (((x)<<24) & 0xff000000))


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define SYS_LIL_ENDIAN

#endif



#endif