/*!
 * \brief Cliente para receber mensagem codificada, decodificar e responder ao servidor 
 * \date 18/04/2016
 * \author Diogo Morgado <diogo.morgado@gmail.com.br>
 */

#include "client.h"

/*!
 * \brief Seta socket como nonblocking
 *
 * param[out] sockfd socket que sera setado como nonblock
 */

int set_nonblock(int sockfd)
{
  int flags;

  if ((flags = fcntl(sockfd, F_GETFL, 0)) == -1)
    flags = 0;

  return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

/*!
 * \brief Cria o socket e conecta ele ao endereco especificado
 * 
 * \return Descritor do socket
 */

static int socket_connect()
{
  int ret = 0, sockfd = 0;
  struct addrinfo hints, *servinfo, *aux;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((ret = getaddrinfo(IP_ADDR, HTTP_PORT, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "Getaddrinfo error: %s\n", gai_strerror(ret));
    return -1;
  }

  for (aux = servinfo; aux != NULL; aux = aux->ai_next)
  {
    if ((sockfd = socket(aux->ai_family, aux->ai_socktype,
         aux->ai_protocol)) == -1)
    {
      fprintf(stderr, "Socket error: %s\n", strerror(errno));
      continue;
    }

    if (connect(sockfd, aux->ai_addr, aux->ai_addrlen) == -1)
    {
      close(sockfd);
      fprintf(stderr, "Connnect error: %s\n", strerror(errno));
      continue;
    }

    break;
  }
  
  freeaddrinfo(servinfo);
  return sockfd;
}

/*
 * A tabela de decoficacao foi feita da seguinte maneira, o indice do 
 * vetor e' o valor em decimal do nibble de entrada(decodificado), e 
 * o valor do indice e' o nibble de saida (codificado). 
 * Por exemplo: A ultima entrada da tabela e' o 15 (1111) e o valor
 * correspondente a ele e' o 11101 (29). No indice 29 se encontra
 * o valor 15.
 *
 */
static int decode_table(int bits)
{  
  int decode[31] = {0,0,0,0,0,0,0,0,0,1,4,5,0,0,6,7,0,0,8,9,2,3,10,11,0,0,12,13,14,15,0};
  return decode[bits]; 
}

static int decode_values(packet *pkt)
{   
//  pkt->de_pack = calloc(1, sizeof(*pkt->de_pack));
  pkt->de_pack.de_nibble.n1 = decode_table(pkt->en_pack->b2);
  pkt->de_pack.de_nibble.n2 = decode_table(pkt->en_pack->b1);
  pkt->de_pack.de_nibble.n3 = decode_table(pkt->en_pack->b4);
  pkt->de_pack.de_nibble.n4 = decode_table(pkt->en_pack->b3);
  pkt->de_pack.de_nibble.n5 = decode_table(pkt->en_pack->b6);
  pkt->de_pack.de_nibble.n6 = decode_table(pkt->en_pack->b5);
  pkt->de_pack.de_nibble.n7 = decode_table(pkt->en_pack->b8);
  pkt->de_pack.de_nibble.n8 = decode_table(pkt->en_pack->b7);
return 0;
}

static int invert_packet(packet *pkt)
{
  int i;
  unsigned char tmp;
  for (i=0;i<2;i++)
  {
    tmp=pkt->encoded_packet[i];
    pkt->encoded_packet[i] = pkt->encoded_packet[4 - i];
    pkt->encoded_packet[4 - i] = tmp;
  }
return 0;
}

static packet *add_packet(packet_list *pkt_list)
{
  packet *pkt;
  pkt = calloc(1, sizeof(*pkt));
  if (pkt == NULL)
    return NULL;
  if (pkt_list->head == NULL)
    pkt_list->head = pkt;
  else
  {
    packet *i = pkt_list->head;
    while (i->next)
      i = i->next;
    i->next = pkt;
  }
  pkt_list->list_len++;
  return pkt;
}

/*!
 * \brief Copia toda a mensagem enviada pelo servidor  
 * 
 * \param[in] sockfd Descritor do socket
 * \param[out] buffer Variavel que contem a mensagem da resposta do HTTP GET
 * 
 * \return 0 se for OK
 * \return -1 se der algum erro
 */
static int get_server_message(packet_list *pkt_list, int sockfd)
{
  int nread = 0, finish = 0;
  add_packet(pkt_list);
  packet *pkt = pkt_list->head;
  do
  {
    if ((nread = recv(sockfd, pkt->full_packet, PACKETSIZE, 0)) <= 0)
      return -1;
    memcpy(pkt->encoded_packet, pkt->full_packet + 1, PACKETSIZE - 2);  
    if (strchr(pkt->full_packet,END_TRANS) != NULL)
      finish++;
    pkt = add_packet(pkt_list);
  } while (!finish);
  return 0;
}

static int concatenate_bytes(char *message, decoded *de_msg)
{
//  memcpy(message, bytes->message, sizeof(bytes->message));
  strcat(message, (const char *) de_msg->message.byte1); 
  strcat(message, (const char *) de_msg->message.byte2); 
//  strcat(message, (char *) msg->byte3); 
 // strcat(message, (char *) msg->byte4);
return 0; 
}

static int decode_message(packet_list *pkt_list)
{
  pkt_list->message = calloc(1, (sizeof(char) * pkt_list->list_len * 4)); //4 bytes para cada pacote convertido
  if (pkt_list->message == NULL)
    return -1;
  
  packet *pkt = pkt_list->head;
  while (pkt != NULL)
  {
    invert_packet(pkt);
    pkt->en_pack = (encoded *) &pkt->encoded_packet;
    decode_values(pkt);
    concatenate_bytes(pkt_list->message, &pkt->de_pack);
    pkt = pkt->next;    
  }
return 0;
}
static int encode_message(packet *pkt)
{
return 0;
}

static int send_message(packet *pkt)
{
return 0;
}
int main()
{ 
  int sockfd = 0; 
  packet_list pkt_list;
  
  memset(&pkt_list, 0, sizeof(pkt_list));

  if ((sockfd = socket_connect()) < 0)
    goto error;

  if (set_nonblock(sockfd) < 0)
    goto error;
  
  if (get_server_message(&pkt_list, sockfd) < 0)
    goto error;
  
  if (decode_message(&pkt_list) < 0)
    goto error;
    
  if (encode_message(pkt_list.head) < 0)
    goto error;  
  
  if (send_message(pkt_list.head) < 0)
    goto error;

  close(sockfd);
  return 0;
      
error:    
    if (sockfd)
      close(sockfd);
  return -1;
}


