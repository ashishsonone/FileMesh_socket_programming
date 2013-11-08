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
#define MYADDR "10.3.132.142"
#define MYPORT 6000

using namespace std;

void receive(int server_fd, char *fpath){ //fpath is where to save the received file(including the name)
    cout << "path to download is : " << fpath <<endl;
    char recvBuff[MTU];  
    memset(recvBuff, '\0', sizeof(recvBuff));

    FILE  *wf;
    cout << "open before\n";
    wf = fopen(fpath, "wb");
    cout << "open after\n";

    int filesize, toreceive; //toreceive keeps track of how much else is 
                             //there to receive before connectin can be closed

    //get first chunk = 1500
    int n = recv(server_fd, recvBuff, sizeof(recvBuff)-1, 0);
    if(n<0){
        cout << "error receiving file" << endl;
        return;
    }
    cout << "received bytes" << n <<endl;
    //get file size
    memcpy(&filesize, recvBuff, sizeof(int));
    filesize=ntohl(filesize);
    cout << "filesize is " << filesize <<endl;

    fwrite(recvBuff+4, 1, n-4, wf); //write to file except first 4 bytes

    toreceive = filesize + 4 - n; //how much more to receive yet
    //get the rest of file
    while(toreceive > 0){
        n = recv(server_fd, recvBuff, sizeof(recvBuff)-1, 0);
        cout << "received bytes" << n <<endl;
        fwrite(recvBuff, 1, n, wf);
        toreceive -= n;
    }
    cout <<"file received ...now closing the socket" <<endl;
    close(server_fd);
    fclose(wf);  //close file object (VERY IMP)
}

void send(int server_fd, char *fpath){
    FILE *rf;
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
    cout <<"file sent ...now closing the socket" <<endl;
    close(server_fd);
    fclose(rf); //close file object (VERY IMP)
}

int main(int argc, char *argv[]) //argv is the node index
{

    map<int, struct sockaddr_in> Mesh; //Mesh map, just to ease which node to send the request just using node index
    Mesh = mesh_configure_client();

    int my_fd, tcp_fd;
    struct sockaddr_in my_addr, their_addr;
    unsigned int addr_len; // NOTE unsigned int

    unsigned short int index = atoi(argv[1]); //this is the index 

    //set my_addr
    my_addr.sin_family = AF_INET;
    //my_addr.sin_addr.s_addr = inet_addr(MYADDR);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(MYPORT+index);
    memset(&(my_addr.sin_zero), '\0', 8); //set rem to nulls
    print_addr(my_addr);

    addr_len = sizeof(struct sockaddr); //size of sockaddr struct. used in sendto() and recvfrom()

    //following are the send and receive buffers
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
        char fpath[50]; //path of file to be sent or received
        char folderloc[50];//folder where file received must be saved
        char *md5sum = new char [50]; //md5sum of file

        cout << "Enter first node id(to be contacted first) : "; cin>>nodeid;
        cout << "Enter option[0 to send, 1 to receive] : "; cin>>code;
        if(code == 0){
            cout << "Enter file path : "; cin>>fpath;
            md5sum = md5_hash(fpath); //calculate md5sum of given file
            cout << "md5 hash is : "<<md5sum<<endl; 
        }
        else if(code == 1){
            cout << "Enter folder where to save : "; cin>>folderloc;
            cout << "Enter md5sum of file : "; cin >> md5sum;
            sprintf(fpath, "%s/%s", folderloc, md5sum); //join `folderloc/md5sum` to get complete file path
        }
        else{
            cout <<"sorry ... wrong option ...exiting ....";
            exit(0);
        }

        //sendto()
        their_addr = Mesh[nodeid]; //send to server nodeid
        cout << "their address " ;
        print_addr(their_addr);

        struct udp_header head; // prepare the header of msg to be sent
        head.req_code = htons(code); // request code (0->upload/send; 1->download/receive)
        head.src_port = my_addr.sin_port; // client's port
        head.src_ip = my_addr.sin_addr.s_addr; // client's ip address
        memcpy(head.md5sum, md5sum, 32); // copy md5sum to head.md5sum

        memcpy(sendBuff, &head, sizeof(head)); //fill the sendBuff and sendto() using udp
        int n = sendto(my_fd, sendBuff, sizeof(struct udp_header), 0,
                        (struct sockaddr*)&their_addr, addr_len);
        if(n>0){
            printf("\t\ttransmitted to %s\n", inet_ntoa(their_addr.sin_addr));
        }
        else{
            cout << "\t\ttransmit error\n";
        }
        
        //listen() : now listen to tcp socket and wait for the right node to contact you
        if(listen(tcp_fd, 5) == -1){
            printf("Failed to listen\n");
            return -1;
        }

        cout <<"waiting for connection" <<endl;
        //accept() : wait for reply from correct node and accept the connection
        int temp_fd = accept(tcp_fd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
        cout << "temp fd : " << temp_fd <<endl;

        //call appropriate function send/receive
        if(code==0) send(temp_fd, fpath);
        else if(code==1) receive(temp_fd, fpath);
        
    //close sockets
    close(my_fd);    
    close(tcp_fd);    
    return 0;
}
