# Httpd

>基于C语言实现的TinyHttpd服务器(http://sourceforge.net/projects/tinyhttpd/)，采用线程解析请求，进程处理响应，以及网络编程技术，与HTTP消息格式等

#### 20180729

1).支持以下请求

```
GET:
	http://localhost:10000
	http://localhost:10000/index.html
	http://localhost:10000/not.html
	http://localhost:10000/index.sh?a=1

POST:
	http://localhost:10000/index.sh
```