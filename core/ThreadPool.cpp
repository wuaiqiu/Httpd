#include "ThreadPool.h"

namespace httpd {

	//线程池初始化
	ThreadPool::ThreadPool() {
		pool =new pool_t;
		auto queue =new queue_t;
		queue->head = nullptr;
		queue->used= 0;
		pool->queue = queue;
		pthread_mutex_init(&pool->mutex, nullptr);
		pthread_cond_init(&pool->cond, nullptr);
		for (int i = 0; i < MAX_THREAD_NUM; i++ ) {
			pthread_create(&pool->tids[i], nullptr,worker,(void*)this);
		}
	}

	//进队
	void ThreadPool::queue_push(queue_t* pqueue, void* pdata){
	    auto *pnode = new node_t;
	    pnode->data = pdata;
	    pnode->next = nullptr;
	    node_t *tmp = pqueue->head;
		if(tmp == nullptr){
			pqueue->head = pnode;
			pqueue->used++;
		}else{
			while(tmp->next!=nullptr)tmp=tmp->next;
			tmp->next=pnode;
			pqueue->used++;
		}
	}

	//出队
	void* ThreadPool::queue_pop(queue_t* pqueue){
		if(pqueue->used==0)return nullptr;
		node_t* phead = pqueue->head;
	    void* data = phead->data;
	    pqueue->head = phead->next;
	    pqueue->used--;
		delete phead;
	    return data;
	}

	//线程池中线程循环执行任务
	void* ThreadPool::worker(void* __this)  {
	    auto _this = static_cast<ThreadPool*>(__this);
	    while (true) {
	    	routine* prt = nullptr;
	    	pthread_mutex_lock(&_this->pool->mutex);
	    	if (_this->pool->queue->used==0) {
	    		  pthread_cond_wait(&_this->pool->cond, &_this->pool->mutex);
	    	}
	    	prt = static_cast<routine*>(_this->queue_pop(_this->pool->queue));
	    	pthread_mutex_unlock(&_this->pool->mutex);
	    	prt->callback(prt->args);
	    }
	    pthread_exit((void*)0);
	}


	//添加工作到列队
	void ThreadPool::pool_routine_add(void (*callback)(void *), void *args){
	    auto *prt = new routine;
	    prt->callback = callback;
	    prt->args = args;
	    pthread_mutex_lock(&pool->mutex);
	    queue_push(pool->queue, prt);
	    pthread_cond_signal(&pool->cond);
	    pthread_mutex_unlock(&pool->mutex);
	}

	//线程池销毁
	ThreadPool::~ThreadPool() {
		for (int i = 0; i < MAX_THREAD_NUM; i++ ){
			pthread_cancel(pool->tids[i]);
		}
		pthread_mutex_destroy(&pool->mutex);
		pthread_cond_destroy(&pool->cond);
		delete pool->queue;
	    delete pool;
	}

}
