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

/*

char str[] = "query=testword&diskimg=simple.img";

sscanf(str, "%*[^=]%*c%[^&]%*[^=]%*c%s", buf1, buf2);
It's pretty cryptic, so here's the summary: each % designates the start of a chunk of text. 
If there's a * following the %, that means that we ignore that chunk and don't store it in one 
of our buffers. The ^ within the brackets means that this chunk contains any number of characters 
that are not the characters within the brackets (excepting ^ itself). %s reads a string of arbitrary
length, and %c reads a single character.
*/
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
