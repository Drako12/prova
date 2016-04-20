#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

#define BUFSIZE BUFSIZ
#define HTTP_PORT "50000"
#define IP_ADDR "54.94.159.157" 
#define PACKETSIZE 7

typedef enum packet_type_
{
  START = 0xC6,
  END_PACKET = 0x6B,
  END_TRANS = 0x21

}packet_type; 

typedef struct packed
{
  unsigned int b1: 5;
  unsigned int b2: 5;
  unsigned int b3: 5;
  unsigned int b4: 5;
  unsigned int b5: 5;
  unsigned int b6: 5;
  unsigned int b7: 5;
  unsigned int b8: 5;
}packed;

typedef union all
{
  packed fields;
  long int all_values;
}all;

typedef struct server_message_
{
  char packet[PACKETSIZE];
  char encoded_packet[PACKETSIZE - 2];
  all  u_pack;
  char buffer[BUFSIZE];
  char decode_buf[BUFSIZE];
  int bytes_read;
  int sockfd;
}server_message;


#endif

