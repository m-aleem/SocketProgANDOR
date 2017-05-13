#include "server_or.h"
using namespace std;

#define MYPORT "3493"
#define EDGE_PORT "3491"
#define MAXBUFLEN 3000

int main(void)
{
   while(1) // Keep the server running, until the user manually terminates
   {
      prog();
   }
}

int prog()
{
   int socketFileDescr;
   struct addrinfo setUpHints, * connecInfo, * p;
   int numbytes;
   struct sockaddr_storage their_addr;
   char buf[MAXBUFLEN];
   socklen_t addr_len;
   vector < string > data;

   // Set up our UDP port

   // Block below from Beej
   memset( & setUpHints, 0, sizeof setUpHints);
   setUpHints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
   setUpHints.ai_socktype = SOCK_DGRAM;
   setUpHints.ai_flags = AI_PASSIVE; // use my IP

   getaddrinfo(NULL, MYPORT, & setUpHints, & connecInfo);

   // Loop through the results and connect when possible (Based on Beej)
   for (p = connecInfo; p != NULL; p = p -> ai_next)
   {
      socketFileDescr = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol);
      bind(socketFileDescr, p -> ai_addr, p -> ai_addrlen);
      break;
   }

   freeaddrinfo(connecInfo); // From Beej

   printf("The Server OR is up and running using UDP on port %s.\n", MYPORT);

   int numLines = 0;
   bool done = false;

   while (!done) // receive all lines for computation
   {
      addr_len = sizeof their_addr; // From Beej
      numbytes = recvfrom(socketFileDescr, buf, MAXBUFLEN - 1, 0, (struct sockaddr * ) & their_addr, & addr_len);
      buf[numbytes] = '\0';

      numLines++;
      if(numLines==1)
      {
         printf("The Server OR start receiving lines from the edge server for OR computation. The computation results are:\n");
      }

      if (strncmp(buf, "InputOver", 9) == 0)
      {
         done = true;
         printf("The Server OR has successfully received %d lines from the edge server and finished all AND computations.\n", numLines - 1);
      }
      else
      {
         string first;
         string second;
         string prefix;
         int pos;
         string str(buf);
         string fullComp;

         prefix = str.substr(0, 7); // header information (###:and,)
         str = str.substr(7, str.length()); // actual numbers

         pos = str.find(',', 0);
         second = str.substr(pos + 1); // second number after ","
         first = str.substr(0, pos); // first number is before ","

         bitset < 10 > i;
         if (first.find_first_of("1", 0) != string::npos)
         {
            i = bitset < 10 > (first); // if it not all zeroes convert first number to bitset
         }

         bitset < 10 > j;
         if (second.find_first_of("1", 0) != string::npos)
         {
            j = bitset < 10 > (second); // if it not all zeroes convert second number to bitset
         }

         bitset < 10 > k = i | j; // compute bit-wise or
         string kStr = k.to_string(); // conver to string for output

          // output computation to screen and put the line, with the prefix, into vector 
         if (kStr != "0000000000")
         {
            cout << first << " or " << second << " = " << kStr.erase(0, kStr.find_first_not_of('0')) << endl;
            fullComp = prefix.substr(0, 4) + first + " or " + second + " = " + kStr;
            data.push_back(fullComp);
         }
         else
         {
            int len = (first.length() > second.length()) ? second.length() : first.length();
            string s(len, '0');
            cout << first << " or " << second << " = " << s << endl;
            fullComp = prefix.substr(0, 4) + first + " or " + second + " = " + s;
            data.push_back(fullComp);
         }
      }
   }

   close(socketFileDescr); // close socket - done receiving

   // loop through all computed lines and send to them back to the edge server
   for (int i = 0; i < data.size(); i++)
   {
      sendBack(data[i]);
   }

   sendBack("InputOrOver"); // send messaget to edge server indicating and computations are complete
   printf("The Server OR has successfully finished sending all computation results to the edge server.\n");

   return 0;

}

// Sends the input string the edge server
int sendBack(string str)
{
   int socketFileDescr;
   struct addrinfo setUpHints, * connecInfo, * p;
   int numbytes;

   // Block below from Beej
   memset( & setUpHints, 0, sizeof setUpHints);
   setUpHints.ai_family = AF_UNSPEC;
   setUpHints.ai_socktype = SOCK_DGRAM;

   getaddrinfo("127.0.0.1", EDGE_PORT, & setUpHints, & connecInfo);

   // Loop through and make a socket (Based on Beej)
   for (p = connecInfo; p != NULL; p = p -> ai_next)
   {
      socketFileDescr = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol);
      break;
   }

   const void * msg = str.c_str(); // convert in order to send
   unsigned len = str.length();

   numbytes = sendto(socketFileDescr, msg, len, 0, p -> ai_addr, p -> ai_addrlen);
   freeaddrinfo(connecInfo);
   close(socketFileDescr);
   
   return 0;
}

