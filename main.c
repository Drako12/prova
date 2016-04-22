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
 * o valor 15
 *
 */
static int decode_table(int bits)
{  
  int decode[31] = {0,0,0,0,0,0,0,0,0,1,4,5,0,0,6,7,0,0,8,9,2,3,10,11,0,0,12,13,14,15,0};
  return decode[bits]; 
}

static int encode_table(int bits)
{
  int code[16] = {30,9,20,21,10,11,14,15,18,19,22,23,26,27,28,29};
  return code[bits];
}
static int decode_values(packet *pkt)
{   
  pkt->de_pack.nibble.n1 = decode_table(pkt->en_tmp_->b2);
  pkt->de_pack.nibble.n2 = decode_table(pkt->en_tmp_->b1);
  pkt->de_pack.nibble.n3 = decode_table(pkt->en_tmp_->b4);
  pkt->de_pack.nibble.n4 = decode_table(pkt->en_tmp_->b3);
  pkt->de_pack.nibble.n5 = decode_table(pkt->en_tmp_->b6);
  pkt->de_pack.nibble.n6 = decode_table(pkt->en_tmp_->b5);
  pkt->de_pack.nibble.n7 = decode_table(pkt->en_tmp_->b8);
  pkt->de_pack.nibble.n8 = decode_table(pkt->en_tmp_->b7);
return 0;
}

static int encode_values(packet *pkt)
{   
  pkt->en_pack.encoded_tmp.b1 = encode_table((pkt->decoded_packet[0] & HIGH_NIBBLE) >> 4);
  pkt->en_pack.encoded_tmp.b2 = encode_table(pkt->decoded_packet[0] & LOW_NIBBLE);
  pkt->en_pack.encoded_tmp.b3 = encode_table((pkt->decoded_packet[1] & HIGH_NIBBLE) >> 4);
  pkt->en_pack.encoded_tmp.b4 = encode_table(pkt->decoded_packet[1] & LOW_NIBBLE);
  pkt->en_pack.encoded_tmp.b5 = encode_table((pkt->decoded_packet[2] & HIGH_NIBBLE) >> 4);
  pkt->en_pack.encoded_tmp.b6 = encode_table(pkt->decoded_packet[2] & LOW_NIBBLE);
  pkt->en_pack.encoded_tmp.b7 = encode_table((pkt->decoded_packet[3] & HIGH_NIBBLE) >> 4);
  pkt->en_pack.encoded_tmp.b8 = encode_table(pkt->decoded_packet[3] & LOW_NIBBLE);

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
  packet *pkt = pkt_list->head;
  do
  {
    pkt = add_packet(pkt_list);
    if ((nread = recv(sockfd, pkt->full_packet, PACKETSIZE, 0)) <= 0)
    {
      fprintf(stderr, "recv error: %s\n", strerror(errno));
      return -1;
    }   
    memcpy(pkt->encoded_packet, pkt->full_packet + 1, PACKETSIZE - 2);  
    if (strchr(pkt->full_packet, END_TRANS) != NULL)
      finish++;

  } while (!finish);
  return 0;
}

static int concatenate_bytes(char *message, decode *msg)
{   
  strncat(message, (const char *) &msg->decoded_message, 4);
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
    pkt->en_tmp_ = (struct en_tmp *) &pkt->encoded_packet;
   // pkt->en_pack.var = pkt->encoded_packet;
    decode_values(pkt);
    concatenate_bytes(pkt_list->message, &pkt->de_pack);
    pkt = pkt->next;    
  }
  return 0;
}
static int trim_spaces(char *message)
{
  char *end_string;
  end_string = message + strlen(message) - 1;
  while (isspace(*end_string))
    end_string--;
  *(end_string + 1) = 0;
return 0;
}

static int invert_message(char *message)
{
  int i, len;
  char tmp;
  len = strlen(message) - 1;
  for (i=0;i<=len/2;i++)
  {
    tmp=message[i];
    message[i] = message[len - i];
    message[len - i] = tmp;
  }
return 0;
}

static int build_packet(packet *pkt)
{ 
  int i;
  pkt->full_en_packet[0] = START;
  if (pkt->next == NULL)
    pkt->full_en_packet[6] = END_TRANS;
  else
    pkt->full_en_packet[6] = END_PACKET;
  
  pkt->full_en_packet[1] = pkt->en_pack.encoded_message.byte5; 
  pkt->full_en_packet[2] = pkt->en_pack.encoded_message.byte4; 
  pkt->full_en_packet[3] = pkt->en_pack.encoded_message.byte3; 
  pkt->full_en_packet[4] = pkt->en_pack.encoded_message.byte2; 
  pkt->full_en_packet[5] = pkt->en_pack.encoded_message.byte1;
  for (i = 0; i <= 5; i++)
  {
    if (pkt->full_en_packet[i] == NULL) 
      pkt->full_en_packet[i] = SPACE;  
  }  
  return 0;
}

static int encode_packets(packet *pkt, char *message)
{
  while (pkt != NULL)
  {    
    memcpy (pkt->decoded_packet, message, 4);      
    encode_values(pkt);
    build_packet(pkt);  
    pkt = pkt->next;
    message = message + 4;    
  }  
  return 0;
}

static int encode_message(packet *pkt, char *message)
{   
  trim_spaces(message);
  printf("Mensagem recebida: %s", message);  
  invert_message(message);
  encode_packets(pkt, message);
  return 0;
}

static int send_message(packet *pkt, int sockfd)
{  
  while (pkt != NULL)
  {
    if (send(sockfd, pkt->full_en_packet, PACKETSIZE, 0) < 0)
    {
    fprintf(stderr, "Send error:%s\n", strerror(errno));
    return -1;
    }
    pkt = pkt->next;
  }  
  return 0;
}

static int receive_confirmation(packet_list *pkt_list, int sockfd)
{ 
  pkt_list->head = NULL; 
  get_server_message(pkt_list, sockfd);
  decode_message(pkt_list);
  trim_spaces(pkt_list->message);
  printf("Confirmacao recebida: %s",pkt_list->message);  
  return 0;
}

int main()
{ 
  int sockfd = 0; 
  packet_list pkt_list;
  
  memset(&pkt_list, 0, sizeof(pkt_list));

  if (set_nonblock(sockfd) < 0)
    goto error;
  
  if ((sockfd = socket_connect()) < 0)
    goto error;
  
  if (get_server_message(&pkt_list, sockfd) < 0)
    goto error;
  
  if (decode_message(&pkt_list) < 0)
    goto error;
    
  if (encode_message(pkt_list.head, pkt_list.message) < 0)
    goto error;  
  
  if (send_message(pkt_list.head, sockfd) < 0)
    goto error;
  
  if (receive_confirmation(&pkt_list, sockfd) < 0)
    goto error;
  
  close(sockfd);
  return 0;
      
error:    
    if (sockfd)
      close(sockfd);
  return -1;
}


