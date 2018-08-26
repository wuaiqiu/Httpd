#include "Response.h"
namespace httpd {

	Response::Response(int client_sock) {
		this->client_sock = client_sock;
	}

	//响应头
	void Response::headers(int status) {
		char buf[1024];
		if (status == 501)
			sprintf(buf, "HTTP/1.0 501 Method Not Implemented \r\n");
		else if (status == 404)
			sprintf(buf, "HTTP/1.0 404 NOT FOUND \r\n");
		else if (status == 400)
			sprintf(buf, "HTTP/1.0 400 BAD REQUEST \r\n");
		else if (status == 500)
			sprintf(buf, "HTTP/1.0 500 Internal Server Error \r\n");
		else
			sprintf(buf, "HTTP/1.0 200 OK \r\n");
		send(client_sock, buf, strlen(buf), 0);
		sprintf(buf, SERVER_STRING);
		send(client_sock, buf, strlen(buf), 0);
		sprintf(buf, "Content-Type: text/html \r\n");
		send(client_sock, buf, strlen(buf), 0);
		sprintf(buf, "\r\n");
		send(client_sock, buf, strlen(buf), 0);
	}

	//501响应
	void Response::method_error() {
		char buf[1024];
		headers(501);
		sprintf(buf, "<h1>HTTP request method not supported</h1>");
		send(client_sock, buf, strlen(buf), 0);
	}

    //404响应
	void Response::not_found() {
		char buf[1024];
		headers(404);
		sprintf(buf, "<h1>Not Found</h1>");
		send(client_sock, buf, strlen(buf), 0);
	}

	//400响应
	void Response::bad_request() {
		char buf[1024];
		headers(400);
		sprintf(buf, "<h1>Your browser sent a bad request</h1>");
		send(client_sock, buf, strlen(buf), 0);
		sprintf(buf, "<h1>such as a POST without a Content-Length</h1>");
		send(client_sock, buf, strlen(buf), 0);
	}

	//500响应
	void Response::cannot_execute() {
		char buf[1024];
		headers(500);
		sprintf(buf, "<h1>Error prohibited CGI execution</h1>");
		send(client_sock, buf, strlen(buf), 0);
	}

    //解析静态文件
	void Response::static_file(const char *realpath) {
		char buf[1024];
		FILE *resource = fopen(realpath, "r");
		headers(200);
		while (!feof(resource)) {
			fgets(buf, sizeof(buf), resource);
			send(client_sock, buf, strlen(buf), 0);
		}
		fclose(resource);
	}

	//解析动态文件
	void Response::dynamic_file(const char *realpath,const char *method,
			const char *query_string, int content_length) {
		int cgi_output[2], cgi_input[2];
		pid_t pid;
		int status, i;
		char c;
		headers(200);
		//建立output管道,input管道
		if (pipe(cgi_output) < 0) {
			cannot_execute();
			return;
		}
		if (pipe(cgi_input) < 0) {
			cannot_execute();
			return;
		}
		//父进程用于收数据以及发送子进程处理的回复数据
		if ((pid = fork()) < 0) {
			cannot_execute();
			return;
		}
		if (pid == 0) {
			char meth_env[255];
			char query_env[255];
			char length_env[255];

			//子进程输出重定向到output管道的1端
			dup2(cgi_output[1], 1);
			//子进程输入重定向到input管道的0端
			dup2(cgi_input[0], 0);
			//关闭无用管道口
			close(cgi_output[0]);
			close(cgi_input[1]);
			//设置CGI环境变量
			sprintf(meth_env, "REQUEST_METHOD=%s", method);
			putenv(meth_env);
			if (strcasecmp(method, "GET") == 0) {
				sprintf(query_env, "QUERY_STRING=%s", query_string);
				putenv(query_env);
			} else {
				sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
				putenv(length_env);
			}
			//执行realpath
			execl("/bin/bash", "bash", realpath, nullptr);
			exit(0);
		} else {
			//关闭无用管道口
			close(cgi_output[1]);
			close(cgi_input[0]);
			if (strcasecmp(method, "POST") == 0) {
				for (i = 0; i < content_length; i++) {
					//得到post请求数据，写到input管道中，供子进程使用
					recv(client_sock, &c, 1, 0);
					write(cgi_input[1], &c, 1);
				}
			}
			//等待子进程返回
			waitpid(pid, &status, 0);
			//从output管道读到子进程处理后的信息，然后send出去
			while (read(cgi_output[0], &c, 1) > 0)
				send(client_sock, &c, 1, 0);
		}
	}

	Response::~Response() {

	}

}
