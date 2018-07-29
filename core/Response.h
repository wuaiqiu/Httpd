#ifndef CORE_RESPONSE_H_
#define CORE_RESPONSE_H_

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/socket.h>

#define SERVER_STRING "Server: Httpd/0.1.0\r\n"

class Response {
public:
	Response();
	void method_error(int client_sock);
	void not_found(int client_sock);
	void bad_request(int client_sock);
	void static_file(int client_sock,const char *realpath);
	void dynamic_file(int client_sock,const char *realpath,const char *method,const char *query_string,int content_length);
	virtual ~Response();

private:
	void headers(int client_sock,int status);
	void cannot_execute(int client_sock);
};

#endif
