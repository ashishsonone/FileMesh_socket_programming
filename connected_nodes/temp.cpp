#include <iostream>
#include <stdio.h> //printf
#include <stdlib.h>//system, NULL
#include <string>
#include <map>
#include <bitset>
#include <openssl/md5.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "header.h"
using namespace std;

struct add{
    unsigned short int port;
    unsigned long ip;
};

map<string, struct add> M;

void mapsetup(){
    struct add x;
    x.port = 33;
    x.ip = 3242;
    M["0"]= x;
    //char hash[100];
    //cout << hash <<endl;
}

int hash(string fpath){
    FILE *in;
    char buff[50];
    //command to be executed formed below
    string cmd = "md5sum ";
    char c_cmd[100];
    cmd = cmd + fpath + " |awk '{print $1}'";
    strcpy(c_cmd, cmd.c_str()); //copy string into a c-style char array

    if(!(in = popen(c_cmd, "r"))){
        return 0;
    }

    while(fgets(buff, sizeof(buff), in)!=NULL){
        cout << buff << " with len " <<strlen(buff) <<endl;
    }

    pclose(in);
    return 0;
}

int main(){
    //testing sprintf
    char c_cmd[100]; 
    int index = 3;
    sprintf(c_cmd, "mkdir -p HARDDISK/node%d", index);
    system(c_cmd);

    //testing env
    cout << getenv("PATH");
    udp_header x;
    mapsetup();
    hash("node.cpp");

    cout << "--------checking md5_hash() and findmodulo() functions\n";
    char *hashed = md5_hash("node.cpp");
    cout << "md5_hash is : " << hashed <<endl;
    unsigned int mod = findmodulo(hashed, 13);
    cout << "hash mod n is : " << mod <<endl;
    cout << "--------END OF checking md5_hash() and findmodulo() functions\n";


    map<int, struct sockaddr_in> CM;
    CM = cluster_setup();
    cout <<"\n" << ntohs(CM[0].sin_port) << " "<< ntohs(CM[1].sin_port) <<endl;
}
