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
unsigned short int nodeindex ; //this is the node # (global variable)
char folderloc[100]; //this is the folder where this node will store files
int numnodes;


void handle_send(udp_header client, char *md5sum){
    int temp_fd; //a temporary socket for handling this particular file transfer connection
    
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = client.src_ip;
    client_addr.sin_port = client.src_port;
    memset(&(client_addr.sin_zero), '\0', 8); //set rem to nulls
    print_addr(client_addr);

    //socket()
    if((temp_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
      perror("socket");
      exit(1);
    }

    printf("\t\ttemporary socket retrieve success\n");
    //connect()
    if(connect(temp_fd, (struct sockaddr *)&client_addr, sizeof(client_addr))<0)
    {
      printf("\n Error : Connect Failed \n");
      return ;
    }
    
    char fpath[100]; //path to file
    sprintf(fpath, "%s/%s", folderloc, md5sum);

    FILE *rf;
    char sendBuff[MTU];
    int fsize;

    rf = fopen(fpath, "rb");
    fseek(rf, 0, SEEK_END); //fseek(fp, offset, origin) -- go to that pos
    fsize = ftell(rf); //ftell(fp) -- tell the current pos
    fseek(rf, 0 , SEEK_SET);//go to begining of file
    cout << "fsize " <<fsize <<endl;
    int fsizenet = htonl(fsize);//fsize in network byte order
    memcpy(sendBuff, &fsizenet, sizeof(int));
    int n =send(temp_fd, sendBuff, sizeof(int), 0);
    if(n<0) cout<<"Send error file";

    while(fsize > 0){
        int len = 1490;
        if(fsize<1490){
            len = fsize;
        }
        fsize -= len;
        fread(sendBuff, 1, len, rf);
        send(temp_fd, sendBuff, len, 0);
    }
    close(temp_fd);
    fclose(rf); //close file object (VERY IMP)
}

void handle_receive(udp_header client, char *md5sum){
    int temp_fd; //a temporary socket for handling this particular file transfer connection
    
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = client.src_ip;
    client_addr.sin_port = client.src_port;
    memset(&(client_addr.sin_zero), '\0', 8); //set rem to nulls
    print_addr(client_addr);

    //socket()
    if((temp_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
      perror("socket");
      exit(1);
    }

    printf("\t\ttemporary socket retrieve success\n");
    //connect()
    if(connect(temp_fd, (struct sockaddr *)&client_addr, sizeof(client_addr))<0)
    {
      printf("\n Error : Connect Failed \n");
      return ;
    }

    char recvBuff[MTU];  
    memset(recvBuff, '0', sizeof(recvBuff));

    /* file location info */   
    //first create the save loc folder if not exists
    char c_cmd[100]; 
    sprintf(c_cmd, "mkdir -p %s", folderloc);
    system(c_cmd);

    char c_path[100];
    sprintf(c_path, "%s/%s", folderloc, md5sum);
    /**********************/

    FILE  *wf;
    wf = fopen(c_path, "wb");

    int filesize, toreceive; //toreceive keeps track of how much else is 
                             //there to receive before connectin can be closed
    //get first chunk = 1500
    int n = recv(temp_fd, recvBuff, sizeof(recvBuff)-1, 0);
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
        n = recv(temp_fd, recvBuff, sizeof(recvBuff)-1, 0);
        cout << "received bytes" << n <<endl;
        fwrite(recvBuff, 1, n, wf);
        toreceive -= n;
    }
    cout <<"file received ...now closing the socket" <<endl;
    close(temp_fd);
    fclose(wf);  //close file object (VERY IMP)
}

int main(int argc, char *argv[]) //argv is the node index
{

    int my_fd;
    struct sockaddr_in my_addr, their_addr;
    unsigned int addr_len; // NOTE unsigned int

    nodeindex = atoi(argv[1]); //this is the index which identifies this node

    map<int, struct sockaddr_in> Mesh; //cluster map containing nodeindex->sockaddr_in mapping
    Mesh = mesh_configure(nodeindex, &numnodes, folderloc);
    cout << "folder loc is " << folderloc << endl;
    cout << "no of nodes is " << numnodes << endl;

    my_addr = Mesh[nodeindex];
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
  
    

    while(true){
        //receivefrom()
        cout <<"waiting for udp receive" <<endl;
        int n = recvfrom(my_fd, recvBuff, sizeof(recvBuff)-1, 0,
                        (struct sockaddr*)&their_addr, &addr_len);
        struct udp_header head; // receive udp request info
        char md5sum[33]; //md5sum of 33 chars with last char NULL, so it can be used as string
        if(n>0){
            recvBuff[n] = 0;
            cout << "received bytes : " << n <<endl;
            // memcpy(void *dest, void* src, size_t numbytes);
            cout << "size of head bytes : " << sizeof(struct udp_header) <<endl;
            memcpy(&head, recvBuff, sizeof(struct udp_header)); //extract udp_header information
            cout << "decoding struct head... port " << ntohs(head.src_port) <<" code "<<ntohs(head.req_code)<<endl;
            memcpy(md5sum, head.md5sum, 32);
            md5sum[32] = 0;
            cout <<"md5sum received is " << md5sum <<endl;
        }
        else{
            cout << "\t\treceive error\n";
        }

        int req_code = ntohs(head.req_code);
        int targetnode = findmodulo(md5sum, numnodes); // this is the index of correct node acc to (hash)%(numnodes)
        bool i = (targetnode==nodeindex);
        cout << "checking target node" << targetnode <<endl;

        // decide based on code whether to forward it to correct server or serve it
        if(i){    // serve the source (whose details in head)
            if(req_code==0){ // client wants to upload a file, so receive and store that file
                handle_receive(head, md5sum);
            }
            else if(req_code==1){ // client wants to download a file, so send that file
                handle_send(head, md5sum);
            }
        }
        else{//forward what was received to appropriate node
            cout << "forwarding to correct node" << req_code ;
            their_addr = Mesh[targetnode];
            memcpy(sendBuff, &head, sizeof(head));
            n = sendto(my_fd, sendBuff, sizeof(struct udp_header), 0,
                            (struct sockaddr*)&their_addr, addr_len);
        }
    }
    close(my_fd);
    return 0;
}
