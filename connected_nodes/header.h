#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h> 
#include <string.h> //for memset
#include <fstream> //for ifstream
#include <math.h>

#define ADDR "127.0.0.1"
#define PORT 5000
#define CONFIGFILE "FileMesh.cfg"

using namespace std;

struct udp_header{
    unsigned short int req_code; // request code-> 0 : store;  1 : retrieve
    unsigned short int src_port; //port Network byte order
    unsigned int src_ip; //32-bit ip address source(client) Network byte order
    char md5sum[32];
};

//foll is struct used to pass data to thread
struct thread_data{
    struct udp_header head; // receive udp request header
    char md5sum[33]; //md5sum of 33 chars with last char NULL
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

/*
 * Read configure file CONFIGFILE
 *
 * returns the map containing mesh info as sockaddr_in
 * Also takes as argument :
 *      int nodeindex      (current node's index)
 *      int* numnodes      (fill it with the no of nodes)
 *      char *folderloc    (fill it with folder info for given nodeindex where it must be save files)
 */
map<int, struct sockaddr_in> mesh_configure(int nodeindex, int *numnodes, char* folderloc){
    map<int, struct sockaddr_in> M; //this is the map to be returned
    string line;
    ifstream infile(CONFIGFILE);
    int index=0;

    while( std::getline( infile, line ) ){
        //std::cout<<line<<'\n';
        char ip[20], folder[100];
        int lport;
        unsigned short int port;
        sscanf(line.c_str(),"%[^:]%*[:]%d%*[ ]%s", ip, &lport, folder); //get ip, port, and folder from the line
        port = lport; //copy to unsigned short int(which port should be)

        struct sockaddr_in my_addr;
        my_addr.sin_family = AF_INET;
        my_addr.sin_addr.s_addr = inet_addr(ip); //the extracted ip string
        my_addr.sin_port = htons(port); //the extracted port
        memset(&(my_addr.sin_zero), '\0', 8); //set rem to nulls

        M[index] = my_addr; //Map index -> this sockaddr_in

        //Now check if this index is that of give nodeindex if yes the set folderloc
        if(index == nodeindex){
            strcpy(folderloc, folder); //copy , dont just set the pointer as `folder` is local variable
        }
        index++;
    }
    *numnodes = index; //simply no of lines in the config file
    return M;
}

map<int, struct sockaddr_in> mesh_configure_client(){//return mesh-map for client
    int numnodes; char folderloc[100]; //just dummy variables to call mesh_configure()
    return mesh_configure(0, &numnodes, folderloc);
}

void print_addr(sockaddr_in addr){ //just to print sockaddr_in struct
    cout << "addr " << inet_ntoa(addr.sin_addr) <<":" << ntohs(addr.sin_port)<< endl;
}


//FINDING MD5SUM AND CALCULATING ITS MODULO

/*
 * find modulo given 128bit md5sum as hex string and integer n
 */
unsigned int findmodulo(char *hex, unsigned int n){     unsigned int p16 = pow(2,16); //2^16
    unsigned int p32modn = ((p16%n)*(p16%n))%n; //2^16 mod n
    //cout << "2^32 mod "<< n <<" = " << p32modn <<endl;
    unsigned int h0,h1,h2,h3; //these are the 4 32-bit integers corr to each 32-bit block of 128-bit number.
                              //hi is the ith block from LSB
    sscanf(hex, "%8x%8x%8x%8x",&h3,&h2,&h1,&h0);
    //cout << h3 << " "<<h2 <<endl;
    //cout << h1 << " "<<h0 <<endl;
    unsigned int r3, r2, r1, r0;
    r3 = ((h3%n)*p32modn*p32modn*p32modn)%n;
    r2 = ((h2%n)*p32modn*p32modn)%n;
    r1 = ((h1%n)*p32modn)%n;
    r0 = (h0%n);
    unsigned int result = (r3+r2+r1+r0)%n;
    return result;
}

/*following function calculates md5sum of a file
 * input : filepath
 * output : md5sum(file)
 */
char* md5_hash(char* fpath){
    FILE *in;
    char md5[50]; //get md5sum string in this char array
    char *result = new char[50];

    //command to be executed formed below : md5sum filepath | awk '{print $1}'
    //gives md5sum as 32 char string
    string cmd = "md5sum ";
    char c_cmd[100];
    sprintf(c_cmd, "md5sum %s |awk '{print $1}'", fpath);

    if(!(in = popen(c_cmd, "r"))){ //execute the system command and get the output in FILE *in
        return 0;
    }

    //copy the output into md5 from FILE* in
    while(fgets(md5, sizeof(md5), in)!=NULL){
        //printf("%s", md5);
    }

    //cout << "md5 sum is" << md5 <<endl;
    strcpy(result, md5); //copy to result
   
    pclose(in);
    return result;
}
