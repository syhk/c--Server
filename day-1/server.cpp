#include<sys/socket.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<cstring>

int main()
{
	// 参数一：IP 地址类型; AF_INET -> IPv4   AF_INET6 -> IPv6
	// 参数二：数据传输方式; SOCK_STREAM 流格式，面向连接 -> TCP ,数据报格式，无连接 -> UPD
	// 参数三：协议; 0 表示根据前面的两个参数自动推导协议类型：TCP/UDP
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	// 将套接字绑定到一个 IP 地址和端口上 
	struct sockaddr_in serv_addr;
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family= AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(8888);
	// 将 socket 地址与文件描述符绑定
	bind(sockfd, (sockaddr*)&serv_addr , sizeof(serv_addr));
  // 监听 socket 端口；参数二：最大监听队列长度
	listen(sockfd, SOMAXCONN);

	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_len = sizeof(clnt_addr);
	bzero(&clnt_addr,sizeof(clnt_addr));
	// 接受客户端连接，accept 函数会阻塞当前程序，直到有一个客户端 socket 被接受程序后才会往下运行
	int clnt_sockfd = accept(sockfd,(sockaddr*)&clnt_addr,&clnt_addr_len);
	printf("new client fd %d! IP: %s Port: %d\n",clnt_sockfd, inet_ntoa(clnt_addr.sin_addr),ntohs(clnt_addr.sin_port));
	return 0;
}

