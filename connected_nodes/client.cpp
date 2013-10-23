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
void send(int server_fd, char *fpath){
    FILE *rf, *wf;
    char sendBuff[MTU];
    int fsize;

    rf = fopen(fpath, "rb");
    fseek(rf, 0, SEEK_END); //fseek(fp, offset, origin) -- go to that pos
    fsize = ftell(rf); //ftell(fp) -- tell the current pos
    fseek(rf, 0 , SEEK_SET);//go to begining of file
    int fsizenet = htonl(fsize);//fsize in network byte order
    memcpy(sendBuff, &fsizenet, sizeof(int));
    int n =send(server_fd, sendBuff, sizeof(int), 0);
    if(n<0) cout<<"Send error file";

    while(fsize > 0){
        int len = 1490;
        if(fsize<1490){
            len = fsize;
        }
        fsize -= len;
        fread(sendBuff, 1, len, rf);
        send(server_fd, sendBuff, len, 0);
    }
    close(server_fd);
    fclose(rf); //close file object (VERY IMP)
}

int main(int argc, char *argv[]) //argv is the node index
{

    int my_fd, tcp_fd;
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

    //socket() for tcp connections (file transfer)
    if((tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
      perror("socket stream");
      exit(1);
    }
    if((my_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
      perror("socket dgram");
      exit(1);
    }
    printf("\t\tsockets retrieve success\n");
 
    ///set SO_REUSEADDR socket option on tcp_fd(so to reuse the port and bind doesnot give error)
    int optval = 1;
    setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    
    //bind() tcpfd to my_addr that will be used to exchange files
    if(bind(tcp_fd, (struct sockaddr*)&my_addr, sizeof(my_addr))<0){
        cout << "bind error"<<endl;
        exit(2);
    }

    /*//wait for tcp connection from server and then upload the file
    if(listen(tcp_fd, 5) == -1){
        printf("Failed to listen\n");
        return -1;
    }*/

    //while(true){
        unsigned short int code, nodeid;
        char fpath[50];char *md5sum;
        cout << "Enter node id:"; cin>>nodeid;
        cout << "Enter code:"; cin>>code;
        cout << "Enter file path:"; cin>>fpath;
        md5sum = md5_hash(fpath);
        cout << "md5 hash is : "<<md5sum<<endl; 
        //sendto()
        map<int, struct sockaddr_in> CM; //cluster map containing nodeindex->sockaddr_in mapping
        CM = cluster_setup(); //use only now just to send to servers easily
        their_addr = CM[nodeid]; //send to server 1
        cout << "their address " ;
        print_addr(their_addr);

        struct udp_header head;
        head.req_code = htons(code);
        head.src_port = my_addr.sin_port;
        head.src_ip = my_addr.sin_addr.s_addr;
        memcpy(head.md5sum, md5sum, 32); //copy md5sum to head.md5sum

        memcpy(sendBuff, &head, sizeof(head)); //fill the sendBuff
        int n = sendto(my_fd, sendBuff, sizeof(struct udp_header), 0,
                        (struct sockaddr*)&their_addr, addr_len);
        if(n>0){
            printf("\t\ttransmitted to %s\n", inet_ntoa(their_addr.sin_addr));
        }
        else{
            cout << "\t\ttransmit error\n";
        }
        
        if(listen(tcp_fd, 5) == -1){
            printf("Failed to listen\n");
            return -1;
        }

        cout <<"waiting for connection" <<endl;
        //wait for reply from correct server
        int temp_fd = accept(tcp_fd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
        cout << "temp fd" << temp_fd <<endl;

        send(temp_fd, fpath);
        
    //}
    close(my_fd);    
    close(tcp_fd);    
    return 0;
}
