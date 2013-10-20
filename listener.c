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

#define MYPORT 5000
#define MYADDR "10.3.131.150"
//#define MYADDR "127.0.0.1"
using namespace std;
 
int main(void)
{
  int myfd = 0;
  
  struct sockaddr_in my_addr;
  struct sockaddr_in their_addr;
  unsigned int addr_len; // NOTE unsigned int
  addr_len = sizeof(struct sockaddr);
 
  char recvBuff[1025];  
  int numrv;  
 
  if((myfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
      perror("socket");
      exit(1);
  }
  printf("socket retrieve success\n");
  
  memset(recvBuff, '0', sizeof(recvBuff));
      
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = inet_addr(MYADDR);
  my_addr.sin_port = htons(MYPORT);
  memset(&(my_addr.sin_zero), '\0', 8); //set rem to nulls
 
  bind(myfd, (struct sockaddr*)&my_addr, sizeof(my_addr));
  
  int n,c; 
    //NOTE THE USE OF RECVFROM
    n = recvfrom(myfd, recvBuff, sizeof(recvBuff)-1, 0,
                    (struct sockaddr*)&their_addr, &addr_len);
    if(n>0)
    {
      recvBuff[n] = 0;

      printf("got packet from %s\n", inet_ntoa(their_addr.sin_addr));
      printf("packet is  %d bytes\n", n);
      if(fputs(recvBuff, stdout) == EOF){
          printf("\n Error : Fputs error");
      }
      printf("\n");
    }
    else{
        cout << "receive error\n";
    }
 
    //now listener sends and talker listens
    cout << "\n... now sending ... \n";
    strcpy(recvBuff, "My First words !!");
    n = sendto(myfd, recvBuff, sizeof(recvBuff)-1, 0,
                    (struct sockaddr*)&their_addr, addr_len); //no pointer in addr_len, its just int
                                    //their_addr and addr_len filled by recv() function when first msg received
    if(n>0){
        printf("transmitted to %s\n", inet_ntoa(their_addr.sin_addr));
    }
    else{
        cout << "transmit error\n";
    }

    close(myfd);    
  return 0;
}
