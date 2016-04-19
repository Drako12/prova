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
#define PACKETSIZE 8

typedef enum packet_type_
{
  START = 0xC6,
  END = 0x6B,
  TRANS = 0x21

}packet_type; 

typedef struct server_message_
{
  char packet[PACKETSIZE];
  char buffer[BUFSIZE];
  int bytes_read;
  int sockfd;
}server_message;


#endif

