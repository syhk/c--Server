#include<sys/socket.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<cstring>
#include<unistd.h>
#include"util.h"

// 通常来说当一个系统调用返回 -1 ，说明有 error 发生
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

	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_len = sizeof(clnt_addr);
	bzero(&clnt_addr,sizeof(clnt_addr));


	int clnt_sockfd = accept(sockfd,(sockaddr*)&clnt_addr,&clnt_addr_len);
	errif(clnt_sockfd == -1 , "socket accept error");

	printf("new client fd %d! IP: %s Port: %d\n",clnt_sockfd, inet_ntoa(clnt_addr.sin_addr),ntohs(clnt_addr.sin_port));


	while(true){
		char buf[1024]; // 定义缓冲区
		bzero(&buf,sizeof(buf)); // 清空缓冲区
		ssize_t read_bytes = read(clnt_sockfd,buf,sizeof(buf)); //从客户端 socket 读到缓冲区，返回已读数据大小
		if(read_bytes > 0){
			printf("message form  client fd %d: %s \n",clnt_sockfd,buf);
			write(clnt_sockfd,buf,sizeof(buf)); // 将相同的数据写回到客户端
		}else if(read_bytes == 0){ // read 返回 0 表示  EOF
			printf("client fd %d disconnected\n",clnt_sockfd);
			close(clnt_sockfd);
			break;
		}else if(read_bytes == -1){ // 返回 -1 ，表示发生错误
			close(clnt_sockfd);
			errif(true,"socket read error");
		}
	close(sockfd);
	}
	return 0;

}

