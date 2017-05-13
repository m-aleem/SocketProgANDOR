#include "edge.h"
using namespace std;

#define TCP_PORT "3490" // My TCP
#define UDP_PORT "3491" // My UDP
#define SERVER_AND_PORT "3492"
#define SERVER_OR_PORT "3493"
#define MAXBUFLEN 3000

int main(void)
{
   while(1) // Keep the server running, until the user manually terminates
   {
      prog();
   }
}

/* This function is based on the function from Beej's Guide to Network Programming */
void preventZombieProc(int s)
{
   int erronoNoOverwrite = errno;
   while (waitpid(-1, NULL, WNOHANG) > 0);
   errno = erronoNoOverwrite;
}

/* This function is based on the function from Beej's Guide to Network Programming */
void * get_IP(struct sockaddr * socketadr)
{
   if (socketadr -> sa_family == AF_INET6) //IP v6
   {
      return &(((struct sockaddr_in6 * ) socketadr) -> sin6_addr);
   }

   return &(((struct sockaddr_in * ) socketadr) -> sin_addr); // Otherwise IP v4
}

int prog()
{
	  int sockfd;
	  int sockfd_tcp;
	  int new_fd; // listen on sock_fd, new connection on new_fd

	  struct addrinfo setUpHints, * connecInfo, * p;
	  struct sockaddr_storage their_addr; // connector's address information
	  socklen_t sin_size;
	  struct sigaction sa;
	  int yes = 1;
	  char s[INET6_ADDRSTRLEN];

	  // Block below from Beej
	  memset( & setUpHints, 0, sizeof setUpHints);
	  setUpHints.ai_family = AF_UNSPEC;
	  setUpHints.ai_socktype = SOCK_STREAM;
	  setUpHints.ai_flags = AI_PASSIVE; // use my IP


	  // SET UP OUR TCP 
	  getaddrinfo(NULL, TCP_PORT, & setUpHints, & connecInfo);

	  // Loop through the results and connect when possible (Based on Beej)
	  for (p = connecInfo; p != NULL; p = p -> ai_next)
	  {
	  	sockfd_tcp = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol);
	  	setsockopt(sockfd_tcp, SOL_SOCKET, SO_REUSEADDR, & yes, sizeof(int));
	  	bind(sockfd_tcp, p -> ai_addr, p -> ai_addrlen);
	  	break;
	  }

	  freeaddrinfo(connecInfo);

	  listen(sockfd_tcp, 10); // Start listening 

	  // Block below from Beej
	  sa.sa_handler = preventZombieProc; // reap all dead processes
	  sigemptyset( & sa.sa_mask);
	  sa.sa_flags = SA_RESTART;
	  sigaction(SIGCHLD, & sa, NULL);

	  printf("The edge server is up and running.\n");

	  int linesRecv = 0;
	  int andCount = 0;
	  int orCount = 0;
	  char buf[3000];
	  vector < string > andData; // vector that will hold our "and" lines
	  vector < string > orData; // vector that will hold our "or" lines

	  sin_size = sizeof their_addr; // From Beej
	  new_fd = accept(sockfd_tcp, (struct sockaddr * ) & their_addr, & sin_size); // From Beej - begin accepting 

	  inet_ntop(their_addr.ss_family, get_IP((struct sockaddr * ) & their_addr), s, sizeof s);
	  bool done = false;
	  string All; // keep appending our buffer content to this string 
	  while (!done)
	  {
	     int numbytes;
	     numbytes = recv(new_fd, buf, 1000 - 1, 0);

	     buf[numbytes] = '\0';
	     All = All.append(buf);
	     if (All.find("InputOver") != -1) // check if the client is done sending lines 
	     {
	        done = true;
	     }

	  }

	  // Process the lines 
	  vector < string > lines; // vector that will hold all lines
	  stringstream splitter(All);
	  string segment;
	  while (getline(splitter, segment, '\n')) // split by this character
	  {
	     lines.push_back(segment); // add to our lines vector
	     linesRecv++; // update our total lines count
	  }

	  for (int i = 0; i < linesRecv - 1; i++) // go through all lines received and format them (add a line number)
	  {
	     string line = lines[i];
	     if (line.substr(0, 3).compare("and") == 0)
	     {
	        stringstream ss;
	        ss << std::setw(3) << std::setfill('0') << i; // add a line number, 3 digits
	        string num = ss.str();
	        string c = num + ":" + line;
	        andData.push_back(c); // add this to our "and" vector
	        andCount++;
	     }
	     else if (line.substr(0, 2).compare("or") == 0)
	     {
	        stringstream ss;
	        ss << std::setw(3) << std::setfill('0') << i; // add a line number, 3 digits
	        string num = ss.str();
	        string c = num + ":" + line;
	        orData.push_back(c);// add this to our "or" vector
	        orCount++;
	     }
	  }

	  printf("The edge server has received %d lines from the client using TCP over TCP_PORT %s.\n", linesRecv - 1, TCP_PORT);


	  int numbytes;
	  socklen_t addr_len;

	  memset( & setUpHints, 0, sizeof setUpHints);
	  setUpHints.ai_family = AF_UNSPEC;
	  setUpHints.ai_socktype = SOCK_DGRAM;

	/*--------------------------------- Send and lines to Server And over UDP ---------------------------------*/
	  getaddrinfo("127.0.0.1", SERVER_AND_PORT, & setUpHints, & connecInfo);
	  for (p = connecInfo; p != NULL; p = p -> ai_next)
	  {
	     sockfd = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol);
	     break;
	  }

	  // Loop through all our "and" lines and send them
	  for (int i = 0; i < andCount; i++)
	  {

	     unsigned len = andData[i].length();
	     const void * lineOfData = andData[i].c_str();
	     if ((numbytes = sendto(sockfd, lineOfData, len, 0, p -> ai_addr, p -> ai_addrlen)) == -1)
	     {
	        perror("talker: sendto");
	        exit(1);
	     }

	  }
	  close(sockfd);
	  
	/*--------------------------------- Send or lines to Server Or over UDP ---------------------------------*/
	  getaddrinfo("127.0.0.1", SERVER_OR_PORT, & setUpHints, & connecInfo);
	  for (p = connecInfo; p != NULL; p = p -> ai_next)
	  {
	     sockfd = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol);
	     break;
	  }

	  // Loop through all our "or" lines and send them
	  for (int i = 0; i < orCount; i++)
	  {
	     unsigned len = orData[i].length();
	     const void * lineOfData = orData[i].c_str();
	     if ((numbytes = sendto(sockfd, lineOfData, len, 0, p -> ai_addr, p -> ai_addrlen)) == -1)
	     {
	        perror("talker: sendto");
	        exit(1);
	     }

	  }
	  close(sockfd);

	  printf("The edge has successfully sent %d lines to Backend-Server AND.\n", andCount);
	  printf("The edge has successfully sent %d lines to Backend-Server OR.\n", orCount);

	  printf("The edge server start receiving the computation results from Backend-Server OR and Backend-Server AND using UDP over port %s.\n", UDP_PORT);
	  printf("The computation results are:\n");

	  vector < string > computedDataAnd; // vector to receive computed "and" data
	  vector < string > computedDataOr; // vector to receive computed "or" data
	  computedDataAnd = getFromBackEnd(SERVER_AND_PORT); // get computed and data
	  computedDataOr = getFromBackEnd(SERVER_OR_PORT); // get computed or data

	  printf("The edge server has successfully finished receiving all computation results from Backend-Server OR and Backend-Server AND.\n");

	  // Send lines of computed data to the client 
	  for (int i = 0; i < andCount; i++) // send all "and" lines 
	  {
	     const void * lineOfData = (computedDataAnd[i].append("\n")).c_str();
	     unsigned len = computedDataAnd[i].length();
	     send(new_fd, lineOfData, len, 0);
	  }
	  for (int i = 0; i < orCount; i++)// send all "or" lines 
	  {
	     const void * lineOfData = (computedDataOr[i].append("\n")).c_str();
	     unsigned len = computedDataOr[i].length();
	     send(new_fd, lineOfData, len, 0);
	  }
	  send(new_fd, "InputOver", 9, 0); // send a message indicating transmission is over

	  printf("The edge server has successfully finished sending all computation results to the client.\n");

	// clear variables for space 
	orCount = 0;
	andCount  = 0;
	linesRecv = 0;
	memset(buf, 0, 3000);
	computedDataAnd.clear();
	computedDataOr.clear();
	splitter.clear();
	segment = "";
	lines.clear();

	// close sockets
	close(new_fd);
	close(sockfd_tcp);
	return 0;
   
}

// Get the computed data from back-end server (server port input)
vector < string > getFromBackEnd(string server)
{
   int sockfd_backend;
   int sockfd_toSend;
   struct addrinfo setUpHints, * connecInfo, * p;

   int numbytes;
   struct sockaddr_storage their_addr;
   char buf[MAXBUFLEN];
   socklen_t addr_len;
   char s[INET6_ADDRSTRLEN];

   vector < string > data;

   // Block below from Beej
   memset( & setUpHints, 0, sizeof setUpHints);
   setUpHints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
   setUpHints.ai_socktype = SOCK_DGRAM;
   setUpHints.ai_flags = AI_PASSIVE; // use my IP

   // Open UDP port to get from back-end server
   getaddrinfo(NULL, UDP_PORT, & setUpHints, & connecInfo);

   // Loop through all the results and bind when possible (Based on Beej)
   for (p = connecInfo; p != NULL; p = p -> ai_next)
   {
   	sockfd_backend = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol);
   	bind(sockfd_backend, p -> ai_addr, p -> ai_addrlen);
   	break;
   }

   bool done = false;
   int computations = 0;
   int messages = 0;

   // Send a message over UDP to back-end server indicating ready to receive messages
   getaddrinfo("127.0.0.1", server.c_str(), & setUpHints, & connecInfo);
   for (p = connecInfo; p != NULL; p = p -> ai_next)
   {
	sockfd_toSend = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol);
   	break;
   }

   numbytes = sendto(sockfd_toSend, "InputOver", 9, 0, p -> ai_addr, p -> ai_addrlen);
   close(sockfd_toSend);

   // receive computed lines
   while (!done)
   {
      addr_len = sizeof their_addr;
      numbytes = recvfrom(sockfd_backend, buf, MAXBUFLEN - 1, 0, (struct sockaddr * ) & their_addr, & addr_len);
      computations++;

      inet_ntop(their_addr.ss_family, get_IP((struct sockaddr * ) & their_addr), s, sizeof s); // From Beej
      buf[numbytes] = '\0';
      if (strncmp(buf, "InputAndOver", 12) == 0 || strncmp(buf, "InputOrOver", 11) == 0) // check if input is over
      {
         done = true;
         computations--;
         messages++;
      }
      else // output to screen 
      {
         string str(buf);
         cout << str.substr(4) << endl;;
         data.push_back(str);
      }
   }

   close(sockfd_backend);
   return data;
}
