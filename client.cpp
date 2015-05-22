#include<iostream>
#include<errno.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>

#define buf 11
#define BUF_SIZE 1024
using namespace std;

int main(int argc, char* argv[])
{
    int port =19000;
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, "10.198.6.106", &server_address.sin_addr);
    server_address.sin_port = htons(port);
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    int sockudp = socket(PF_INET, SOCK_DGRAM, 0);
    int sockudp2 = socket(PF_INET, SOCK_DGRAM, 0);
    assert(sockfd >= 0);
    if(connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
      cout<<"tcp connect error"<<endl;
      return 1;
    }
    else
    {
      const char* tcp = "This is TCP data.";
      send(sockfd, tcp, strlen(tcp), 0);
      char buf[BUF_SIZE];
      int ret = recv(sockfd, buf, BUF_SIZE - 1, 0);
      if(ret < 0)
      {
        cout<<"recv tcp error"<<endl;
      }
      else
      {
        buf[ret] = '\0';
        cout<<ret<<" "<<buf<<endl;
      }
    }
    if(connect(sockudp, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
      cout<<"udp connect error"<<endl;
      return 1;
    }
    else
    {
      const char* udp = "This is UDP data.";
      int iret = send(sockudp, udp, strlen(udp), 0);
   //   cout << iret << endl;
     // perror("send") ;
      //cout << strerror(errno) << endl;
      char buf[BUF_SIZE];
      int ret = recv(sockudp, buf, BUF_SIZE - 1, 0);
      if(ret < 0)
      {
        cout<<"recv udp error"<<endl;
      }
      else
      {
        buf[ret] = '\0';
        cout<<ret<<" "<<buf<<endl;
      }
    }
   const char* buf = "This is direct UDP data.";
   sendto(sockudp2, buf, strlen(buf), 0, (struct sockaddr*)&server_address, sizeof(server_address)); 
   char bufrcv[BUF_SIZE];
   socklen_t len = sizeof(server_address);
   int retrcv = recvfrom(sockudp2, bufrcv, BUF_SIZE - 1, 0, (struct sockaddr*)&server_address, &len); 
   bufrcv[retrcv] = '\0';
   cout << retrcv  << " " << bufrcv << endl;
   
   close(sockfd);
   return 0;
    
}
