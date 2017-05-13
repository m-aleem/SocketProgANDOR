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

// Socket 
#include <signal.h>

// Input and Output
#include <iostream>

// Use Vectors
#include <vector>

// Format strings 
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

int receiveInput();
vector<string> getFromBackEnd(string server);
int sendToClient(vector<string> data);
int prog();
