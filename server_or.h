#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <sys/wait.h>



// Input and Output
#include <iostream>

// Use Vectors
#include <vector>

// For bitwise computations
#include <bitset>

using namespace std;

int sendBack(string);
int prog();