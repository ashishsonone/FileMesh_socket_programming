#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include <iostream>
using namespace std;
 
int main(void)
{
  int sockfd = 0,n = 0;
  char recvBuff[1024];
  struct sockaddr_in serv_addr;
 
  memset(recvBuff, '0' ,sizeof(recvBuff));
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
      printf("\n Error : Could not create socket \n");
      return 1;
    }
 
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(5000);
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
 
  if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
      printf("\n Error : Connect Failed \n");
      return 1;
    }
 
  //while loop to receive indefinitely from server
  while((n = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0)) > 1)
    {
      cout << n << endl;
      recvBuff[n] = 0;
      if(fputs(recvBuff, stdout) == EOF){
          printf("\n Error : Fputs error");
      }
      cout << "received " << n << endl;
      printf("\n");
    }

  cout << "out of the loop " << endl;
  //reply back to server
  char sendBuff[200];
 
  strcpy(sendBuff, "Reply from client");
  int c = send(sockfd, sendBuff, strlen(sendBuff), 0);
  cout << "sent " << c << endl;

  if( n < 0)
    {
      printf("\n Read Error \n");
    }
 
 
  return 0;
}
