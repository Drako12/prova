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

static int socket_connect(server_message *message)
{
  int ret = 0;
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
    if ((message->sockfd = socket(aux->ai_family, aux->ai_socktype,
         aux->ai_protocol)) == -1)
    {
      fprintf(stderr, "Socket error: %s\n", strerror(errno));
      continue;
    }

    if (connect(message->sockfd, aux->ai_addr, aux->ai_addrlen) == -1)
    {
      close(message->sockfd);
      fprintf(stderr, "Connnect error: %s\n", strerror(errno));
      continue;
    }

    break;
  }
  
  freeaddrinfo(servinfo);
  return 0;
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

static int decode_values(packed *pack)
{
  unpacked p;
  p.unpack_n.u1= decode_table(pack->b2);
  p.unpack_n.u2= decode_table(pack->b1);
  p.unpack_n.u3= decode_table(pack->b4);
  p.unpack_n.u4= decode_table(pack->b3);
  p.unpack_n.u5= decode_table(pack->b6);
  p.unpack_n.u6= decode_table(pack->b5);
  p.unpack_n.u7= decode_table(pack->b8);
  p.unpack_n.u8= decode_table(pack->b7);  
}
static int read_packet(server_message *message)
{
   int i;
   unsigned char tmp;
   unsigned char teste[7]={0xC6, 0x57, 0x54, 0x95, 0x5E, 0x9E, 0x6B};
   unsigned char teste2=0x57;
   memcpy(message->encoded_packet, teste + 1, PACKETSIZE - 2); 
   for (i=0;i<2;i++)
   {
     tmp=message->encoded_packet[i];
     message->encoded_packet[i] = message->encoded_packet[4 - i];
     message->encoded_packet[4 - i] = tmp;
   }
   message->en_pack = &message->encoded_packet;
  decode_values(message->en_pack);
  //for (i = 0;i < 5; i++)
  //  message->u_pack[i].all_values = message->encoded_packet[i];
  //packed *p = (packed *) message->encoded_packet;
     

// memcpy(message->encoded_packet, message->packet + 1, PACKETSIZE - 2); 
    // packed *p = (packed * ) message->encoded_packet;    
  return 0;
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
static int get_server_message_and_decode(server_message *message)
{
  //int nread = 0;
  read_packet(message);
  /*do
  {
    if ((nread = recv(message->sockfd, message->buffer + message->bytes_read, PACKETSIZE, 0)) <= 0)
      return -1;
    memcpy(message->packet, message->buffer + message->bytes_read, PACKETSIZE);
    message->bytes_read += nread;
  } while (read_packet(message) == 0);*/
  return 0;
}


int main()
{  
  server_message message;
    
  memset(&message, 0, sizeof(message));

  //if (socket_connect(&message) < 0)
//    goto error;

  //if (set_nonblock(message.sockfd) < 0)
    //goto error;
  
  if (get_server_message_and_decode(&message) == -1)
   goto error;
  
  //if (decode_message($message) == -1)
  // goto error;
   

  //printf("Mensagem %04x", message.buffer);
 
  close(message.sockfd);
  return 0;
      
error:    
    if (message.sockfd)
      close(message.sockfd);
  return -1;
}


