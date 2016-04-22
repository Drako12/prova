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
#include <ctype.h>
#define BUFSIZE BUFSIZ
#define HTTP_PORT "50002"
#define IP_ADDR "54.94.159.157" 
#define PACKETSIZE 7
#define HIGH_NIBBLE 240
#define LOW_NIBBLE 15

typedef enum packet_type_
{
  START = 0xC6,
  END_PACKET = 0x6B,
  END_TRANS = 0x21,
  SPACE = 0x20
}packet_type; 

struct __attribute__((packed)) en_tmp
{
  unsigned int b8: 5;
  unsigned int b7: 5;
  unsigned int b6: 5;
  unsigned int b5: 5;
  unsigned int b4: 5;
  unsigned int b3: 5;
  unsigned int b2: 5;
  unsigned int b1: 5;
};

struct __attribute__((packed)) en_msg 
{
  unsigned char byte1: 8;
  unsigned char byte2: 8;
  unsigned char byte3: 8;
  unsigned char byte4: 8;
  unsigned char byte5: 8;
};

struct __attribute__((packed)) de_msg 
{
    unsigned int byte1: 8;
    unsigned int byte2: 8;
    unsigned int byte3: 8;
    unsigned int byte4: 8;
};

struct __attribute__((packed)) de_nibble
{
  unsigned int n1: 4;
  unsigned int n2: 4;
  unsigned int n3: 4;
  unsigned int n4: 4;
  unsigned int n5: 4;
  unsigned int n6: 4;
  unsigned int n7: 4;
  unsigned int n8: 4; 
};

typedef union encode_
{
  unsigned long var;
  unsigned long int whole: 40;
  struct en_tmp encoded_tmp;
  struct en_msg encoded_message;
}encode;

typedef union decode_
{
  unsigned long var;
  struct de_msg decoded_message;
  struct de_nibble nibble;
  struct en_tmp encoded_tmp;  
}decode;
  
typedef struct packets_
{
  encode en_pack;
  decode de_pack;
  struct en_tmp *en_tmp_;
  unsigned char encoded_packet[PACKETSIZE - 2];
  char full_packet[PACKETSIZE];
  char full_en_packet[PACKETSIZE];
  unsigned char decoded_packet[PACKETSIZE - 3];
  struct packets_ *next;
}packet;

typedef struct packet_list_
{
  int list_len;
  char *message;
  char *en_message;
  packet *head;
}packet_list; 


#endif

