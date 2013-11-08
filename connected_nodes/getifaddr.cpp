#include <stdio.h>
#include <iostream>

#include "header.h"
using namespace std;

int main(){
    char *iface = "eth0";
    char *ip = getipaddr(iface);
    cout << "FINAL IP IS "<< ip <<endl;
    cout << "mod is " << findmodulo("3c159506e3b0056e67f75b2daa52a9fa", 10);
}
