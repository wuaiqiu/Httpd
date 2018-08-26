#include "core/Request.h"

using namespace httpd;

int main(){
	Request* r = new Request;
	r->startup(10000);
	return 0;
}
