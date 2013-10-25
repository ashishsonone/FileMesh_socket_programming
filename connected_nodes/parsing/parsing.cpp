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
#include <fstream> //for ifstream
using namespace std;

int main(){
    string line;
    ifstream infile("test.txt");
    int index=0;
    while( std::getline( infile, line ) ){
        //std::cout<<line<<'\n';
        char ip[20], folder[100];
        int lport;
        unsigned short int port;
        sscanf(line.c_str(),"%[^:]%*[:]%d%*[ ]%s", ip, &lport, folder);
        port = lport; //copy to unsigned short int(which port should be)
        cout << "node#" << index << " "<<ip << " len " << strlen(ip) <<"  folder is `" <<folder << "` len "<<strlen(folder) << endl;
        cout << port <<endl;
        index++;
    }

    infile.close();
}
