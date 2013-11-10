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

void receive(int server_fd, char *fpath){ //fpath is where to save the received file(including the name)
    cout << "path to download is : " << fpath <<endl;
    char recvBuff[MTU];  
    memset(recvBuff, '\0', sizeof(recvBuff));

    FILE  *wf;
    wf = fopen(fpath, "wb");

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
    //Now get the rest of file
    int printcount = 0; //used to print the status of transfer every 100 receives
    while(toreceive > 0){
        n = recv(server_fd, recvBuff, sizeof(recvBuff)-1, 0);
        fwrite(recvBuff, 1, n, wf);
        toreceive -= n;
        if(printcount++ ==100){ //when toreceive count reaches 100 print status, and reset the count
            printcount = 0;
            cout << "receive operation  .. " <<((float)(filesize -toreceive)*100)/filesize << "% complete \r";
            cout.flush();
        }
    }
    cout << "receive operation  .. " <<((float)(filesize -toreceive)*100)/filesize << "% complete \n";
    cout <<"file received ...now closing the socket" <<endl;
    close(server_fd);
    fclose(wf);  //close file object (VERY IMP)
}

void send(int server_fd, char *fpath){
    FILE *rf;
    char sendBuff[MTU];
    int fsize, FileSize; //fsize will hold #bytes yet to be sent, FileSize will always hold size of file

    rf = fopen(fpath, "rb");
    fseek(rf, 0, SEEK_END); //fseek(fp, offset, origin) -- go to that pos
    fsize = ftell(rf); //ftell(fp) -- tell the current pos
    FileSize = fsize;
    cout << "FileSize is  " << fsize <<endl;
    fseek(rf, 0 , SEEK_SET);//go to begining of file
    int fsizenet = htonl(fsize);//fsize in network byte order
    memcpy(sendBuff, &fsizenet, sizeof(int));
    int n =send(server_fd, sendBuff, sizeof(int), 0);
    if(n<0) cout<<"Send error file";

    int printcount = 0; //used to print the status of transfer every 100 receives
    while(fsize > 0){
        int len = 1490;
        if(fsize<1490){ //this takes care of the last chunk which can be less than say 1490 and must be careful while reading from FILE and sending it
            len = fsize; //set len to remaining file length
        }
        if(printcount++ ==100){ //when toreceive count reaches 100 print status, and reset the count
            printcount = 0;
            cout << "trasfer operation  .. " <<((float)(FileSize -fsize)*100)/FileSize << "% complete \r";
            cout.flush();
        }
        fsize -= len;
        fread(sendBuff, 1, len, rf);
        send(server_fd, sendBuff, len, 0);
    }
        cout << "trasfer operation  .. " <<((float)(FileSize -fsize)*100)/FileSize << "% complete \n";
    cout <<"file sent ...now closing the socket" <<endl;
    close(server_fd);
    fclose(rf); //close file object (VERY IMP)
}

/*
 * takes interface name(e.g eth0, eth1) as argument so that it can get the ip address of host
 *
 * follow is a sample command to run at terminal after compiling
 *
 * ./client.out eth1       - here interface name will be taken as eth1
 * ./client.out            - when no interface name given as argument, eth0 is assumed to be default
 */
int main(int argc, char *argv[])
{

    map<int, struct sockaddr_in> Mesh; //Mesh map, just to ease which node to send the request just using node index
    Mesh = mesh_configure_client();

    int my_fd, tcp_fd;
    struct sockaddr_in my_addr, their_addr;
    unsigned int addr_len; // NOTE unsigned int
    char *myinterface = "eth0"; //interface name e.g eth0 (can be an command line argument)

    if(argc > 1){ //i.e inteface name is give
        myinterface = argv[1];
    }

    char *myip = getipaddr(myinterface); //get the ip address using helper funtion defined in header.h
    cout << "my ip is " << myip <<endl;

    //set my_addr
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = inet_addr(myip);
    //my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = 0; //NOTE imp : if set to 0, on bind() system will automatically chose a random free port
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
 
    
    //bind() tcpfd to my_addr that will be used to exchange files
    if(bind(tcp_fd, (struct sockaddr*)&my_addr, sizeof(my_addr))<0){
        cout << "bind error"<<endl;
        exit(2);
    }

    //**NOTE** use getsockname to get the new addr assigned by system to tcp_fd socket, into my_addr struct
    getsockname(tcp_fd, (struct sockaddr*)&my_addr, &addr_len);
    //check the port assigned after bind()
    print_addr(my_addr);

    //while(true){
        unsigned short int code, nodeid;
        char fpath[500]; //path of file to be sent or received
        char folderloc[500];//folder where file received must be saved
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
