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

using namespace std;

int main(int argc, char *argv[]) //argv is the node index
{

    int my_fd;
    struct sockaddr_in my_addr, their_addr;
    unsigned int addr_len; // NOTE unsigned int

    map<int, struct sockaddr_in> CM; //cluster map containing nodeindex->sockaddr_in mapping
    CM = cluster_setup();

    unsigned short int index = atoi(argv[1]); //this is the index used to get own sockaddr_in from CM
    my_addr = CM[index];
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
  
    

    //receive
    int n = recvfrom(my_fd, recvBuff, sizeof(recvBuff)-1, 0,
                    (struct sockaddr*)&their_addr, &addr_len);
    struct udp_header head; //receive udp request info
    if(n>0){
        recvBuff[n] = 0;
        cout << "received bytes : " << n <<endl;
        //memcpy(void *dest, void* src, size_t numbytes);
        cout << "size of head bytes : " << sizeof(struct udp_header) <<endl;
        memcpy(&head, recvBuff, sizeof(head));
        cout << "decoding struct head... port " << ntohs(head.src_port) <<" code "<<ntohs(head.req_code)<<endl;
    }
    else{
        cout << "\t\treceive error\n";
    }

    int req_code = ntohs(head.req_code);
    //decide based on code whether to forward it to correct server or serve it
    if(req_code==index){//serve the source (whose details in head)
        cout << "serving the source";
        their_addr.sin_addr.s_addr = head.src_ip;
        their_addr.sin_port = head.src_port;
        char send[] = "lemme serve you. I am THE node"; 
        strcpy(sendBuff, send);
        n = sendto(my_fd, sendBuff, strlen(sendBuff), 0,
                        (struct sockaddr*)&their_addr, addr_len);
    }
    else{//forward what was received to appropriate node
        cout << "forwarding to correct node" << req_code ;
        their_addr = CM[req_code];
        memcpy(sendBuff, &head, sizeof(head));
        n = sendto(my_fd, sendBuff, sizeof(struct udp_header), 0,
                        (struct sockaddr*)&their_addr, addr_len);
    }
    close(my_fd);
    return 0;
}
