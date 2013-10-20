#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <iostream>

#define THEIRPORT 5000
#define OTHER "10.3.131.150"
using namespace std;
 
int main(void)
{
  int myfd = 0;
  
  struct sockaddr_in my_addr;
  struct sockaddr_in their_addr;
  unsigned int addr_len; // NOTE unsigned int
  addr_len = sizeof(struct sockaddr);
 
  char sendBuff[1025];  
  int numrv;  
 
  if((myfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
      perror("socket");
      exit(1);
  }
  printf("socket retrieve success\n");
  
  memset(sendBuff, '0', sizeof(sendBuff));
  strcpy(sendBuff, "My First words !!");
      
  //NO BINDING exclusively specified, system will use any of the unused port
  //bind(myfd, (struct sockaddr*)&my_addr, sizeof(my_addr));

  //BUT their addr must be filled
  their_addr.sin_family = AF_INET;
  their_addr.sin_addr.s_addr = inet_addr(OTHER);
  their_addr.sin_port = htons(THEIRPORT);
  memset(&(their_addr.sin_zero), '\0', 8); //set rem to nulls
  
  int n,c; 
    n = sendto(myfd, sendBuff, sizeof(sendBuff)-1, 0,
                    (struct sockaddr*)&their_addr, addr_len); //no pointer in addr_len, its just int
    if(n>0){
        printf("transmitted to %s\n", inet_ntoa(their_addr.sin_addr));
    }
    else{
        cout << "transmit error\n";
    }
 
    //Now talker receives from listener
    cout << "\n... now receiving ... \n";
    n = recvfrom(myfd, sendBuff, sizeof(sendBuff)-1, 0,
                    (struct sockaddr*)&their_addr, &addr_len);
    if(n>0)
    {
      sendBuff[n] = 0;

      printf("got packet from %s\n", inet_ntoa(their_addr.sin_addr));
      printf("packet is  %d bytes\n", n);
      if(fputs(sendBuff, stdout) == EOF){
          printf("\n Error : Fputs error");
      }
      printf("\n");
    }
    else{
        cout << "receive error\n";
    }
    close(myfd);    
  return 0;
}
