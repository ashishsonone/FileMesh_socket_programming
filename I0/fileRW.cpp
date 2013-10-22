#include <iostream>
#include <cstdio>
#define MTU 1500

using namespace std;

FILE *rf, *wf;
int send(char *buff, int len){
    fwrite(buff, 1, len, wf);
}

int main(){
    char rname[20] = "input.txt";
    char wname[20] = "output.txt";
    char buff[MTU];
    long fsize;

    rf = fopen(rname, "rb");
    wf = fopen(wname, "wb");

    fseek(rf, 0, SEEK_END); //fseek(fp, offset, origin) -- go to that pos
    fsize = ftell(rf); //ftell(fp) -- tell the current pos
    fseek(rf, 0 , SEEK_SET);//go to begining of file
    while(fsize > 0){
        int len = 1500;
        if(fsize<MTU){
            len = fsize;
        }
        fsize -= len;
        fread(buff, 1, len, rf);
        send(buff, len);
    }
    fclose(rf);
    fclose(wf);
}


