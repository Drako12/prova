#include "client.h"

int main()
{ 
  int sockfd = 0; 
  packet_list pkt_list;
  memset(&pkt_list, 0, sizeof(pkt_list));
  
  if ((sockfd = socket_connect()) < 0)
    goto error;
  
  if (get_server_message(&pkt_list, sockfd) < 0)
    goto error;
  
  if (decode_message(&pkt_list) < 0)
    goto error;
    
  encode_message(pkt_list.head, pkt_list.message);
   
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


