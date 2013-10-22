#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <map>

#include <pthread.h> //thread to simultaneous read write
//any extra header files
#include "header.h"

#define MTU 1500
#define MYADDR "127.0.0.1"
#define MYPORT 6000

using namespace std;

int main(int argc, char *argv[]) //argv is the node index
{

    int my_fd;
    struct sockaddr_in my_addr, their_addr;
    unsigned int addr_len; // NOTE unsigned int

    unsigned short int index = atoi(argv[1]); //this is the index 

    //set my_addr
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = inet_addr(MYADDR);
    my_addr.sin_port = htons(MYPORT+index);
    memset(&(my_addr.sin_zero), '\0', 8); //set rem to nulls
    print_addr(my_addr);

    addr_len = sizeof(struct sockaddr);

    char sendBuff[MTU], recvBuff[MTU];  
    memset(sendBuff, '0', sizeof(sendBuff));
    memset(recvBuff, '0', sizeof(recvBuff));

    //socket()
    if((my_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
      perror("socket");
      exit(1);
    }
    printf("\t\tsocket retrieve success\n");
 
    //bind()
    bind(my_fd, (struct sockaddr*)&my_addr, sizeof(my_addr));

    //sendto()
    map<int, struct sockaddr_in> CM; //cluster map containing nodeindex->sockaddr_in mapping
    CM = cluster_setup(); //use only now just to send to servers easily
    their_addr = CM[1]; //send to server 1
    cout << "their address " ;
    print_addr(their_addr);

    struct udp_header head;
    head.req_code = htons(index);
    head.src_port = my_addr.sin_port;
    head.src_ip = my_addr.sin_addr.s_addr;
    memcpy(sendBuff, &head, sizeof(head));
    int n = sendto(my_fd, sendBuff, sizeof(struct udp_header), 0,
                    (struct sockaddr*)&their_addr, addr_len);
    if(n>0){
        printf("\t\ttransmitted to %s\n", inet_ntoa(their_addr.sin_addr));
    }
    else{
        cout << "\t\ttransmit error\n";
    }
    
    //wait for reply from correct server
    n = recvfrom(my_fd, recvBuff, sizeof(recvBuff)-1, 0,
                    (struct sockaddr*)&their_addr, &addr_len);
    if(n>0){
        recvBuff[n] = 0;
        print_addr(their_addr);
        cout << recvBuff <<endl;
    }
    else{
        cout << "\t\treceive error\n";
    }
    close(my_fd);    
    return 0;
}
