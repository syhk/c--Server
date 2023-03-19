#include<sys/socket.h>
#include<arpa/inet.h>
#include<cstring>
#include<iostream>
#include<unistd.h>
#include "util.h"

int main(){

	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	errif(sockfd == -1 , "socket create error");

	struct sockaddr_in serv_addr;
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(8888);

	errif(
	connect(sockfd,(sockaddr*)&serv_addr,sizeof(serv_addr))
	== -1 ,
	"socket connect error"
			);


	while(true){
    // 定义缓冲区	
		char buf[1024];
		// 清空缓冲区
		bzero(&buf,sizeof(buf));
		scanf("%s",buf); // 输入要传到服务器的数据
		ssize_t write_bytes = write(sockfd ,buf,sizeof(buf));
		if(write_bytes == -1){ // 返回 -1，发生了错误
			printf("socket already disconnected , can't write any more\n");
			break;
		}
		bzero(&buf,sizeof(buf)); // 清空缓冲区
		ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
		if(read_bytes > 0 ){
			printf("message form server: %s\n",buf);
		}else if(read_bytes == 0){
			printf("server socket disconnected!\n");
			break;
		}else if(read_bytes == -1){
			close(sockfd);
			errif(true,"socket read error");
		}
	}
	close(sockfd);
	return 0;
}
