#ifndef CORE_THREADPOOL_H_
#define CORE_THREADPOOL_H_

#include "Core.h"

//线程池最大数量
#define MAX_THREAD_NUM 3

namespace httpd {
	//工作列队节点
	struct node_t{
	    node_t* next;
	    void* data;
	};

	//工作列队头
	struct queue_t{
	    node_t *head;
	    int used; //0表示未使用
	};

	//线程池
	struct pool_t{
		 queue_t *queue; //工作列队
		 pthread_mutex_t mutex; //互斥锁
		 pthread_cond_t cond; //条件变量
		 pthread_t tids[MAX_THREAD_NUM];//线程数组
	};

	//执行函数结构体
	struct routine {
		  void *args;
		  void (*callback)(void*);
	};

	class ThreadPool {
	public:
		ThreadPool();
		void pool_routine_add(void (*callback)(void *), void *args);
		virtual ~ThreadPool();

	private:
		pool_t* pool;
		void queue_push(queue_t* pqueue, void* pdata);
		void* queue_pop(queue_t* pqueue);
		static void* worker(void* args);
	};

}

#endif
