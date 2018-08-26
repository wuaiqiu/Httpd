#include "Request.h"

namespace httpd {

	Request::Request() {
		server_sock = -1;
		client_sock = -1;
		client_len = sizeof(client);
		server_len = sizeof(server);
		query_string = nullptr;
		content_length = -1;
		response = nullptr;
		pool = new ThreadPool;
	}

	//启动服务监听端口
	void Request::startup(int port) {
		server_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (server_sock == -1)
			error_die("socket初始化错误");
		memset(&server, 0, server_len);
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		server.sin_addr.s_addr = INADDR_ANY;
		if (bind(server_sock, (sockaddr *) &server, server_len) < 0)
			error_die("bind绑定端口错误");
		if (listen(server_sock, 5) < 0)
			error_die("listen监听端口错误");
		while (true) {
			client_sock = accept(server_sock, (sockaddr *) &client, &client_len);
			if (client_sock == -1)
				error_die("accept解析错误");
			response = new Response(client_sock);
			pool->pool_routine_add(resolve,(void*)this);
		}
		close(server_sock);
	}

	//解析请求线程
	void Request::resolve(void* __this) {
		auto _this = static_cast<Request*>(__this);
		int numchars = 0, cgi = 0;
		size_t i = 0, j = 0;
		char buffer[1024];
		struct stat st;
		numchars = _this->get_line(buffer, sizeof(buffer));
		//解析method
		while (!ISspace(buffer[j]) && (i < sizeof(method) - 1)) {
			_this->method[i] = buffer[j];
			i++;
			j++;
		}
		_this->method[i] = '\0';
		if (strcasecmp(_this->method, "GET") && strcasecmp(_this->method, "POST")) {
			_this->response->method_error();
			close(_this->client_sock);
			pthread_exit((void*) 0);
		}
		if (strcasecmp(_this->method, "POST") == 0)
			cgi = 1;
		//解析path
		i = 0;
		while (ISspace(buffer[j]) && (j < sizeof(buffer)))
			j++;
		while (!ISspace(buffer[j]) && (i < sizeof(_this->path) - 1)
				&& (j < sizeof(buffer))) {
			_this->path[i] = buffer[j];
			i++;
			j++;
		}
		_this->path[i] = '\0';
		//解析query_string
		if (strcasecmp(_this->method, "GET") == 0) {
			_this->query_string = _this->path;
			while ((*(_this->query_string) != '?')
					&& (*(_this->query_string) != '\0')) {
				_this->query_string++;
			}
			if (*(_this->query_string) == '?') {
				cgi = 1;
				*(_this->query_string) *= '\0';
				_this->query_string++;
			}
		}
		//解析realpath
		sprintf(_this->realpath, "htdocs%s", _this->path);
		if (_this->realpath[strlen(_this->realpath) - 1] == '/')
			strcat(_this->realpath, "index.html");
		if (stat(_this->realpath, &st) == -1) {
			_this->response->not_found();
		} else {
			if (!cgi) {
				_this->response->static_file(_this->realpath);
			} else {
				if (strcasecmp(_this->method, "POST") == 0) {
					numchars = _this->get_line(buffer, sizeof(buffer));
					while ((numchars > 0) && strcmp("\n", buffer)) {
						buffer[15] = '\0';
						if (strcasecmp(buffer, "Content-Length:") == 0) {
							_this->content_length = atoi(&(buffer[16]));
							break;
						}
						numchars = _this->get_line(buffer, sizeof(buffer));
					}
					if (_this->content_length == -1) {
						_this->response->bad_request();
						close(_this->client_sock);
						pthread_exit((void*) 0);
					}
				}
				_this->response->dynamic_file(_this->realpath,_this->method,
						_this->query_string, _this->content_length);
			}
		}
		close(_this->client_sock);
	}

	//读取请求头
	int Request::get_line(char *buf, int size) {
		int n = 0, i = 0;
		char c = '\0';
		while ((i < size - 1) && (c != '\n')) {
			n = recv(client_sock, &c, 1, 0);
			if (n > 0) {
				if (c == '\r') {
					n = recv(client_sock, &c, 1, MSG_PEEK);
					if ((n > 0) && (c == '\n'))
						recv(client_sock, &c, 1, 0);
					else
						c = '\n';
				}
				buf[i] = c;
				i++;
			} else {
				c = '\n';
			}
		}
		buf[i] = '\0';
		return i;
	}

	//错误处理
	void Request::error_die(const char *sc) {
		perror(sc);
		exit(1);
	}

	Request::~Request() {
		delete response;
	}
}
