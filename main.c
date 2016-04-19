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

/*!
 * \brief Copia toda a mensagem enviada pelo servidor  
 * 
 * \param[in] sockfd Descritor do socket
 * \param[out] buffer Variavel que contem a mensagem da resposta do HTTP GET
 * 
 * \return 0 se for OK
 * \return -1 se der algum erro
 */
static int get_server_message(server_message *message)
{
  int nread = 0;

  while (strchr(message->packet, (int) END_TRANS) == NULL)
  {
    if ((nread = recv(message->sockfd, message->buffer + message->bytes_read, PACKETSIZE, 0)) <= 0)
      return -1;

    memcpy(message->packet, message->buffer + message->bytes_read, PACKETSIZE);
    message->bytes_read += nread;
  }
  return 0;
}

int main()
{  
  server_message message;
    
  memset(&message, 0, sizeof(message));

  if (socket_connect(&message) < 0)
    goto error;

  if (set_nonblock(message.sockfd) < 0)
    goto error;
  
  if (get_server_message(&message) == -1)
   goto error;

  printf("Mensagem %04x", message.buffer);
 
  close(message.sockfd);
  return 0;
      
error:    
    if (message.sockfd)
      close(message.sockfd);
  return -1;
}


