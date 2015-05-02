#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

static void *thread_routine (void *arg) { //arg is connected fd
	char inbuf[512];

	pthread_detach(pthread_self());

	int connfd = (long int) arg;

	memset(&inbuf, 0, sizeof(inbuf));
	int recvlen=recv(connfd, inbuf, sizeof(inbuf), 0);
	if(recvlen<=0) 
		pthread_exit((void *) 0);

	char cpy[512];
	strcpy(cpy, inbuf);
	char cmd[128];
	strcpy(cmd, strtok(cpy, "\n"));
	printf("%s\n", cmd);

	char ret[2048];
	memset(&ret, 0, sizeof(ret));
	char* token = strtok(cmd, " ");

	if(strcmp(token,"GET")==0){
		token = strtok(NULL, " ");
		FILE * fp;
		if (strcmp(token,"/")==0){
			fp = fopen("./index.html","r");
		} else {
			char fn[128] = ".";
			strcat(fn,token);
			fp = fopen(fn,"r");
		}
		if (fp == NULL ) {
			snprintf(ret,sizeof(ret),
					"HTTP/1.0 404 Not Found\r\n"
					"\r\n"
					"<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>\n");
		} else {
			char file[2048];
			snprintf(ret,sizeof(ret),
					"HTTP/1.0 200 OK\r\n"
					"Content-Type: text/html\r\n"
					"\r\n");
			while(fgets(file, 2048, fp) != NULL) {
				strcat(ret, file);
			}
			fclose(fp);
		}
	} else {
		snprintf(ret,sizeof(ret),
				"HTTP/1.0 400 Bad Request\r\n"
				"\r\n"
				"<html><head><title>400 Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>\n");
	}
	printf("%s\n", ret);
	send(connfd, ret, (int)strlen(ret), 0);

	close(connfd);
	pthread_exit((void *) 0);
}

int main(int argc, char *argv[] ) {
	pthread_t thread_id;
	int portnum = 21600+1029;
	int LISTENQ = 10;
	char *rawaddr = "133.11.206.167";
	int listenfd, accfd;
	struct sockaddr_in server,client;
	socklen_t len;

	server.sin_port = htons(portnum);
	server.sin_family = AF_INET;
	int _i = inet_pton(AF_INET, rawaddr, &server.sin_addr);

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	int _b = bind(listenfd,
			(struct sockaddr *)&server,
			sizeof(server));
	int _l = listen(listenfd,LISTENQ);

	while (1) {
		accfd = accept(listenfd,
				(struct sockaddr *)&client,
				&len);
		if(pthread_create(&thread_id, NULL, thread_routine, (void *)(long int)accfd) != 0) {
			perror("pthread_create");
		}	
	}

}

