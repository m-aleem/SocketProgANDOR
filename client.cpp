#include "client.h"
using namespace std;

#define PORT "3490" // Edge Server TCP Port

/* This function is based on the function from Beej's Guide to Network Programming */
void * get_IP(struct sockaddr * socketadr)
{
   if (socketadr -> sa_family == AF_INET6) //IP v6
   {
      return &(((struct sockaddr_in6 * ) socketadr) -> sin6_addr);
   }

   return &(((struct sockaddr_in * ) socketadr) -> sin_addr); // Otherwise IP v4
}

int main(int argc, char * argv[])
{
   int socketFileDesc;
   int numbytes;
   struct addrinfo addrHints, * connecInfo, * p;
   char addrs[INET6_ADDRSTRLEN]; // From Beej

   // Check that you have the right arguments
   if (argc != 2)
   {
      fprintf(stderr, "Use: ./client <input filename>\n");
      exit(1);
   }

   //Process the input file below
   ifstream infile;
   infile.open(argv[1]); // File name is the first input argument

   vector < string > data; // Store the input file in a vector of strings

   string line;
   while (getline(infile, line))
   {
      data.push_back(line);
   }
   infile.close();

   memset( & addrHints, 0, sizeof addrHints);
   addrHints.ai_family = AF_UNSPEC;
   addrHints.ai_socktype = SOCK_STREAM; // TCP connection 

   getaddrinfo("127.0.0.1", PORT, & addrHints, & connecInfo);

   // Loop through the results and connect when possible
   for (p = connecInfo; p != NULL; p = p -> ai_next)
   {
	  socketFileDesc = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol);
	  connect(socketFileDesc, p -> ai_addr, p -> ai_addrlen);
	  break;
   }

   printf("The client is up and running. \n");

   inet_ntop(p -> ai_family, get_IP((struct sockaddr * ) p -> ai_addr), addrs, sizeof addrs); // From Beej

   int numLines = data.size(); // Lines of data to send

   // Loop through each line of data and send it to the edge server
   for (int i = 0; i < numLines; i++)
   {
      const void * lineOfData = (data[i].append("\n")).c_str(); // append a line break to each line, so that we can split it later
      unsigned len = data[i].length();
      send(socketFileDesc, lineOfData, len, 0); // Send data line
   }
   send(socketFileDesc, "InputOver", 9, 0); // Send a message indicating input is over

   printf("The client has successfully finished sending %d lines to the edge server. \n", numLines);

   // Receive the computed lines from the edge server
   char buf[100];
   bool done = false;
   string All;
   while (!done)
   {
      int numbytes;
      numbytes = recv(socketFileDesc, buf, 99, 0);
      buf[numbytes] = '\0';
      All = All.append(buf);
      if (All.find("InputOver") != -1) // Check if the edge server is done sending lines
      {
         done = true;
      }

   }

   cout << "The client has successfully finished receiving all computation results from the edge server." << endl;
   cout << "The final computation result are:" << endl;

   // Sort the lines back into the original order
   vector < string > lines;
   stringstream splitter(All);
   string segment;
   while (getline(splitter, segment, '\n'))
   {
      lines.push_back(segment);
   }
   sort(lines.begin(), lines.end());

   // Print the final computation results 
   for (int i = 0; i < lines.size(); i++)
   {
      if (lines[i].find("InputOver") == -1)
      {
         cout << lines[i].substr(4) << endl;
      }
   }

   return 0;
}
