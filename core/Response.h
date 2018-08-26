#ifndef CORE_RESPONSE_H_
#define CORE_RESPONSE_H_

#include "Core.h"

#define SERVER_STRING "Server: Httpd/1.0.0 \r\n"
namespace httpd {

	class Response {
	public:
		Response(int client_sock);
		void method_error();
		void not_found();
		void bad_request();
		void static_file(const char *realpath);
		void dynamic_file(const char *realpath, const char *method,
				const char *query_string, int content_length);
		virtual ~Response();

	private:
		int client_sock;
		void headers(int status);
		void cannot_execute();
	};

}
#endif
