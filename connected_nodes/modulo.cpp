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
#include <math.h>
#include "header.h"
using namespace std;

unsigned int findmodulo(char *hex, unsigned int n){ //find modulo given 128bit md5sum as hex string and integer n
    unsigned int p16 = pow(2,16); //2^16
    unsigned int p32modn = ((p16%n)*(p16%n))%n; //2^16 mod n
    cout << "2^32 mod "<< n <<" = " << p32modn <<endl;
    unsigned int h0,h1,h2,h3; //these are the 4 32-bit integers corr to each 32-bit block of 128-bit number.
                              //hi is the ith block from LSB
    sscanf(hex, "%8x%8x%8x%8x",&h3,&h2,&h1,&h0);
    cout << h3 << " "<<h2 <<endl;
    cout << h1 << " "<<h0 <<endl;
    unsigned int r3, r2, r1, r0;
    r3 = ((h3%n)*p32modn*p32modn*p32modn)%n;
    r2 = ((h2%n)*p32modn*p32modn)%n;
    r1 = ((h1%n)*p32modn)%n;
    r0 = (h0%n);
    unsigned int result = (r3+r2+r1+r0)%n;
    return result;
}

//following function calculates md5sum of a file modulo given number `n`
//input : filepath, n
//output : md5sum(file) modulo n
int hash_mod_n(char* fpath, unsigned int n){
    FILE *in;
    char md5[50]; //get md5sum string in buff

    //command to be executed formed below : md5sum filepath | awk '{print $1}'
    //gives md5sum as 32 char string
    string cmd = "md5sum ";
    char c_cmd[100];
    sprintf(c_cmd, "md5sum %s |awk '{print $1}'", fpath);

    if(!(in = popen(c_cmd, "r"))){
        return 0;
    }

    fgets(md5, sizeof(md5), in);
    cout << "md5 sum is" << md5 <<endl;
   
    pclose(in);
    //Now calculate the modulo of md5sum(stored as char[] md5) with `n`
    //use findmodulo() function
    unsigned int mod = findmodulo(md5, n);
    return mod;
}

int main(){
    cout << 1506%13 << endl;
    char x[] = "9cabca53988459ff36537ff332d05bb1";
    unsigned int mod = hash_mod_n("client.cpp", 13);
    cout << mod << " "<<findmodulo(x, 13);
}
