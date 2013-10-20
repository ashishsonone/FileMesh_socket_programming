#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <pthread.h> //thread to simultaneous read write

#include <iostream>

#define THEIRPORT 5000
//#define MYADDR "10.3.131.150"
#define OTHER "127.0.0.1"
using namespace std;

int myfd;
struct sockaddr_in my_addr;
struct sockaddr_in their_addr;
unsigned int addr_len; // NOTE unsigned int

void *listen(void *)
{
    int n;
    char recvBuff[1025];
    memset(recvBuff, '0', sizeof(recvBuff));
    while(true){
        usleep(50000);
        n = recvfrom(myfd, recvBuff, sizeof(recvBuff)-1, 0,
                        (struct sockaddr*)&their_addr, &addr_len);
        if(n>0){
          recvBuff[n] = 0;

          //printf("\t\tgot packet from %s\n", inet_ntoa(their_addr.sin_addr));
          //printf("\t\tpacket is  %d bytes\n", n);
          cout << "\nHim>" ;
          if(fputs(recvBuff, stdout) == EOF){
              printf("\n\t\t Error : Fputs error");
          }
        }
        else{
            cout << "\t\treceive error\n";
        }
          cout << "\nYou>" ;
    }
    pthread_exit(NULL);
}
 
int main(void)
{
  myfd = 0;
  
  addr_len = sizeof(struct sockaddr);
 
  char sendBuff[1025];  
  int numrv;  
 
  if((myfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
      perror("socket");
      exit(1);
  }
  printf("\t\tsocket retrieve success\n");
  
  memset(sendBuff, '0', sizeof(sendBuff));
      
  //BUT their addr must be filled
  their_addr.sin_family = AF_INET;
  their_addr.sin_addr.s_addr = inet_addr(OTHER);
  their_addr.sin_port = htons(THEIRPORT);
  memset(&(their_addr.sin_zero), '\0', 8); //set rem to nulls
  

    pthread_t listenthread;
    int rc = pthread_create(&listenthread, NULL, 
                          listen, NULL);
    
      int n;

    cout << "\n\t\twrite msgs and press RETURN to send ... \n";
    while(true){
        //NOTE THE USE OF RECVFROM
        //now listener sends and talker listens
        //strcpy(sendBuff, "My First words !!");
        usleep(50000);
        cout<<"\nYou>";
        gets(sendBuff);
        n = sendto(myfd, sendBuff, sizeof(sendBuff)-1, 0,
                        (struct sockaddr*)&their_addr, addr_len); //no pointer in addr_len, its just int
                                        //their_addr and addr_len filled by recv() function when first msg received
        if(n>0){
            printf("\t\ttransmitted to %s\n", inet_ntoa(their_addr.sin_addr));
        }
        else{
            cout << "\t\ttransmit error\n";
        }
    }

    close(myfd);    
  return 0;
}
