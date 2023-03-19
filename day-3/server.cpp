#include<sys/socket.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<cstring>
#include<unistd.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<errno.h>
#include"util.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void setnonblocking(int fd){
	fcntl(fd,F_SETFL,fcntl(fd,F_GETFL) | O_NONBLOCK);
}

int main()
{
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	errif(sockfd == -1,"socket create error");

	struct sockaddr_in serv_addr;
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family= AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(8888);
	
	errif( bind(sockfd,(sockaddr*)&serv_addr,sizeof(serv_addr))  == -1 , "socket bind error"  );

	errif(listen(sockfd,SOMAXCONN) == -1 , "socket listen error");
	//参数是一个 flag ，一种设为 0
	int epfd = epoll_create1(0); // 返回一个 epoll 文件描述符
	errif(epfd == -1 , "epoll create error");

	struct epoll_event events[MAX_EVENTS],ev;
	bzero(&events,sizeof(events));

	bzero(&ev,sizeof(ev));
	ev.data.fd = sockfd;
	ev.events = EPOLLIN | EPOLLET;
	setnonblocking(sockfd);
	epoll_ctl(epfd,EPOLL_CTL_ADD/*MOD 修改epoll 红黑树上的事件 DEL 删除事件*/,sockfd,&ev); // 添加事件到 epoll 

//	struct sockaddr_in clnt_addr;
//	socklen_t clnt_addr_len = sizeof(clnt_addr);
//	bzero(&clnt_addr,sizeof(clnt_addr));


//	int clnt_sockfd = accept(sockfd,(sockaddr*)&clnt_addr,&clnt_addr_len);
//	errif(clnt_sockfd == -1 , "socket accept error");

//	printf("new client fd %d! IP: %s Port: %d\n",clnt_sockfd, inet_ntoa(clnt_addr.sin_addr),ntohs(clnt_addr.sin_port));


	while(true){
		int nfds = epoll_wait(epfd,events , MAX_EVENTS,-1);
		errif(nfds == -1 , "epoll wait error");

		for(int i = 0 ; i < nfds ; ++i){
			if(events[i].data.fd == sockfd){ // 新客户端连接
			struct sockaddr_in clnt_addr;
			bzero(&clnt_addr,sizeof(clnt_addr));
			socklen_t clnt_addr_len = sizeof(clnt_addr);

			int clnt_sockfd = accept(sockfd, (sockaddr*)&clnt_addr,&clnt_addr_len);
			errif(clnt_sockfd == -1, "socket accept error");
			printf("new client fd %d! IP: %s Port: %d\n",clnt_sockfd,inet_ntoa(clnt_addr.sin_addr),ntohs(clnt_addr.sin_port));

			bzero(&ev,sizeof(ev));
			ev.data.fd = clnt_sockfd;
			ev.events = EPOLLIN | EPOLLET;
			setnonblocking(clnt_sockfd);
			epoll_ctl(epfd,EPOLL_CTL_ADD,clnt_sockfd,&ev);
			} else if(events[i].events & EPOLLIN){ // 可读事件
				char buf[READ_BUFFER];
			  while(true){ // 由于使用非阻塞 IO ，读取客户端 buffer ，一次读取 buf 大小数据，直到全部读取完成
					 bzero(&buf,sizeof(buf));
					 ssize_t bytes_read = read(events[i].data.fd,buf,sizeof(buf));
					 if(bytes_read > 0){
							printf("message from client fd %d: %s\n",events[i].data.fd,buf);
							write(events[i].data.fd,buf,sizeof(buf));
					 }else if(bytes_read == -1 && errno == EINTR ) // 客户端正常中断，继续读取
					{
							printf("continue reading");
					 		continue;
					}else if(bytes_read == -1 && ((errno == EAGAIN || errno == EWOULDBLOCK)))
					{ // 非阻塞 IO ，这个条件表示数据全部读取完成
						printf("finish reading once, errno : %d \n",errno);
						break;
					}else if(bytes_read == 0){ // EOF ，客户端断开连接
						printf("EOF,client fd %d disconneted\n",events[i].data.fd);
						close(events[i].data.fd);// 关闭 socket 会自动将文件描述符从 epoll 村上移除
						break;
					}
				}
			}else{ 
				printf("something else happend\n");
			}
		}
	}
		close(sockfd);
	return 0;
}

