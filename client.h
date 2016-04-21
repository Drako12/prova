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

typedef struct __attribute__((packed)) encoded_
{
  unsigned int b8: 5;
  unsigned int b7: 5;
  unsigned int b6: 5;
  unsigned int b5: 5;
  unsigned int b4: 5;
  unsigned int b3: 5;
  unsigned int b2: 5;
  unsigned int b1: 5;
}encoded;

typedef union decoded_
{
  struct __attribute__((packed)) de_nibble_
  {
  unsigned int n1: 4;
  unsigned int n2: 4;
  unsigned int n3: 4;
  unsigned int n4: 4;
  unsigned int n5: 4;
  unsigned int n6: 4;
  unsigned int n7: 4;
  unsigned int n8: 4; 
  }de_nibble;
  
  struct __attribute__((packed)) message_ 
  {
    unsigned int byte1: 8;
    unsigned int byte2: 8;
    unsigned int byte3: 8;
    unsigned int byte4: 8;
  }message;
}decoded;

typedef struct packets_
{
  encoded *en_pack;
  decoded *de_pack;
  unsigned char encoded_packet[PACKETSIZE - 2];
  char full_packet[PACKETSIZE];
  unsigned char decoded_packet[PACKETSIZE - 2];
  struct packets_ *next;
}packet;

typedef struct packet_list_
{
  int list_len;
  char *message;
  packet *head;
}packet_list; 


#endif

