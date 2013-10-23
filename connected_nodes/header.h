#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h> 
#include <string.h> //for memset
#define ADDR "127.0.0.1"
#define PORT 5000

using namespace std;

struct udp_header{
    unsigned short int req_code; // request code-> 0 : store;  1 : retrieve
    unsigned short int src_port; //port Network byte order
    unsigned int src_ip; //32-bit ip address source(client) Network byte order
};

map<int, struct sockaddr_in> cluster_setup(){
    map<int, struct sockaddr_in> M;
    for(int i=0; i<6; i++){
        struct sockaddr_in my_addr;
        my_addr.sin_family = AF_INET;
        my_addr.sin_addr.s_addr = inet_addr(ADDR);
        my_addr.sin_port = htons(PORT+i);
        memset(&(my_addr.sin_zero), '\0', 8); //set rem to nulls
        M[i] = my_addr;
    }
    return M;
}

void print_addr(sockaddr_in addr){ //just to print sockaddr_in struct
    cout << "addr " << inet_ntoa(addr.sin_addr) <<":" << ntohs(addr.sin_port)<< endl;
}
