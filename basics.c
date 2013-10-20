#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <cstring>
#include <iostream>
#include <stdio.h>

#include <unistd.h>

#define MYPORT 4242
#define DEST_IP "127.0.0.1"
#define DEST_PORT 23

using namespace std;

int main(){
    struct sockaddr_in myaddr;
    myaddr.sin_family = AF_INET; //host byte order
    myaddr.sin_port = htons(MYPORT);
    //can also use this:  dest_addr.sin_addr.s_addr = inet_addr(DEST_IP);
    inet_aton("10.200.11.12", &(myaddr.sin_addr));
    memset(&(myaddr.sin_zero), '\0', 8); //set rem to nulls

    char *a1 = inet_ntoa(myaddr.sin_addr);
    printf("%s\n", a1);


    cout << printf("%04x",myaddr.sin_addr.s_addr) <<  endl;
    cout << printf("%04x", ntohl(myaddr.sin_addr.s_addr)) << endl;

    //do syscalls
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); //do error check

    //bind(sockfd, (struct sockaddr *)&myaddr, sizeof(struct sockaddr)); //error check bind

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET; //host byte order
    dest_addr.sin_port = htons(MYPORT);
    dest_addr.sin_addr.s_addr = inet_addr(DEST_IP);
    inet_aton(DEST_IP, &(dest_addr.sin_addr));
    memset(&(dest_addr.sin_zero), '\0', 8); //set rem to nulls

    cout << "hello there" <<endl;
    int c= connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
    if(c < 0) cout << "Connect() failed\n";

    char hname[200];
    gethostname(hname, sizeof(hname));
    cout << hname <<endl;

}
