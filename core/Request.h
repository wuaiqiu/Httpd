#ifndef CORE_REQUEST_H_
#define CORE_REQUEST_H_

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
#include "Response.h"

#define ISspace(x) isspace((int)(x))

class Request {
private:
	int server_sock,client_sock;
	sockaddr_in client,server;
	socklen_t client_len,server_len;
	char method[255];
	char path[255];
	char realpath[255];
	char *query_string;
	int content_length;
	Response* response;

public:
	Request();
	void startup(int port);
	virtual ~Request();

private:
	static void* resolve(void* __this);
	int get_line(char *buf, int size);
	void error_die(const char *sc);
};

#endif
