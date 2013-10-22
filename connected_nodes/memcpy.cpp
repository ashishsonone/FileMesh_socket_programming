/* memcpy example */
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "header.h"

using namespace std;

int main ()
{
  unsigned short int i32 = 32;
  cin >> i32;
  char buffer[10];

  unsigned short int i32n = htons(i32);
  cout << "i32 :" << i32 << endl;
  cout << "i32 network byte order  :" << i32 << endl;

  //memcpy(void *dest, void* src, size_t numbytes);
  cout << "size in bytes :" << sizeof(i32n) <<endl;
  memcpy(buffer, &i32n, 2);

  unsigned short int res = 0;
  memcpy(&res, buffer, 2);
  cout << "getting back :" << ntohs(res) << endl;

  udp_header u;
  u.src_port = ntohs(23);
  cout << "size of unsigned short" << sizeof(unsigned short) <<endl;
  cout << "size of unsigned short" << sizeof(unsigned short) <<endl;
  cout << "size of unsigned long" << sizeof(unsigned long) <<endl;
  cout << "size of unsigned long" << sizeof(unsigned int) <<endl;
  struct sockaddr_in ad;
  cout << "size of unsigned long" << sizeof(ad.sin_addr.s_addr) <<endl;
  return 0;
}
