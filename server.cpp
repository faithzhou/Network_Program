#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <iostream>

using namespace std;

#define MAX_EVENT_NUMBER 1024
#define TCP_BUF_SIZE 512
#define UDP_BUF_SIZE 1024

int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void addfd(int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
	cout << "add fd:%d success!" << fd << endl;
}

int main(int argc, char* argv[])
{
	const char* ip = "10.198.6.106";
	int port = 19000;

	int ret = 0;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(listenfd, 5);
	assert(ret != -1);

	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);
	int udpfd = socket(PF_INET, SOCK_DGRAM, 0);
	assert(udpfd >= 0);

	ret = bind(udpfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);

	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	addfd(epollfd, listenfd);
	addfd(epollfd, udpfd);

	while (1)
	{
		//cout << "Start to monitoring!" << endl;
		int number = epoll_wait(epollfd, events, 5, 60);
		//cout << "number is %d!" << number <<  endl;
		if (number < 0)
		{
			cout << "epoll failure" << endl;
			break;
		}
		//cout << "listen fd is " << listenfd << endl;
		//cout << "udp fd is " << udpfd << endl;
		for (int i = 0; i < number; i++)
		{
			int sockfd = events[i].data.fd;
			cout << "\ni = " << i << ";of number " << number << ", its fd is " << sockfd << endl;
			if (sockfd == listenfd)//对方连接
			{
				struct sockaddr_in client_address;
				socklen_t client_address_len = sizeof(client_address);
			//	memset(&client_address, 0, sizeof(struct sockaddr_in));

				int client_fd = accept(listenfd, (struct sockaddr*)&client_address,&client_address_len);
				cout << "******client_fd is " << client_fd << "******" << endl;
				assert(client_fd >= 0);
				addfd(epollfd, client_fd);
				cout << "listen success" << endl;
			}

			else if (sockfd == udpfd)//udp连接接收数据
			{
				char buf[UDP_BUF_SIZE];
				memset(buf, '\0', UDP_BUF_SIZE);
				struct sockaddr_in client_addr;
				//memset(&client_addr, 0, sizeof(struct sockaddr_in));

				socklen_t client_addr_len = sizeof(client_addr);
				int ret = recvfrom(udpfd, buf, UDP_BUF_SIZE - 1, 0, (struct sockaddr*)&client_addr, &client_addr_len);

				if (ret > 0)
				{
					cout << "******udp buffer(size = " << ret << ") is : " << buf << "******" << endl;
					int iret = sendto(udpfd, buf, ret, 0, (struct sockaddr*)&client_addr, client_addr_len);
					cout << "udp send number is " << iret << endl;
				}
				cout << "udp recv success" << endl;
			}

			else if (events[i].events & EPOLLIN)//tcp连接接收数据
			{
				char buf[TCP_BUF_SIZE];
				while (1)
				{
					memset(buf, '\0', TCP_BUF_SIZE);
					ret = recv(events[i].data.fd, buf, TCP_BUF_SIZE - 1, 0);
					if (ret < 0)
					{
						if (errno == EAGAIN || errno == EWOULDBLOCK)
						{
							break;
						}
						close(sockfd);
						break;
					}

					else if (ret == 0)//关闭连接
					{
						close(sockfd);
						break;
					}

					else  //处理数据
					{
						cout << "******tcpfd is :"<< sockfd << ", and recv data is :" << buf << "******" << endl;
						send(sockfd, buf, ret, 0);
					}
					cout << "ret is " << ret << endl;
				}
				cout << "tcp recv success" << endl;
			}

			else
			{
				cout << "unknown error!" << endl;
			}
		}
	}
	close(listenfd);
	return 0;
}
