/*
 * \file: ThreadPool.h
 * \brief: Created by hushouguo at Jul 07 2017 03:13:47
 */
 
#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

BEGIN_NAMESPACE_BUNDLE {
	class ThreadPool
	{
		public:
			typedef std::function<void()> task_type;
	
		public:
			explicit ThreadPool(int n = 0);
	
			~ThreadPool()
			{
				this->stop();
				this->_cond.notify_all();
				this->join();
			}
	
			void stop()
			{
				this->_stop.store(true, std::memory_order_release);
			}

			void join()
			{
				for (auto& thread : this->_threads) {
					if (thread.joinable()) {
						thread.join();
					}
				}
			}
	
			template<class Function, class... Args>
			std::future<typename std::result_of<Function(Args...)>::type> add(Function&&, Args&&...);

		private:
			ThreadPool(ThreadPool&&) = delete;
			ThreadPool& operator = (ThreadPool&&) = delete;
			ThreadPool(const ThreadPool&) = delete;
			ThreadPool& operator = (const ThreadPool&) = delete;
	
		private:
			std::atomic<bool> _stop;
			std::mutex _mtx;
			std::condition_variable _cond;	
			std::queue<task_type> _tasks;
			std::vector<std::thread> _threads;
		};
	
	
		inline ThreadPool::ThreadPool(int n)
			: _stop(false)
		{
			int threads = n;
			if (threads <= 0) {
				threads = std::thread::hardware_concurrency();
			}

			while (threads-- > 0) {
				this->_threads.push_back(std::thread([this]{
					while (!this->_stop.load(std::memory_order_acquire)) {
						task_type task;
						{
							std::unique_lock<std::mutex> locker(this->_mtx);
							this->_cond.wait(locker, [this]{ 
								return this->_stop.load(std::memory_order_acquire) || !this->_tasks.empty();
							});
							if (this->_stop.load(std::memory_order_acquire)) {
								return;
							}
							task = std::move(this->_tasks.front());
							this->_tasks.pop();
						}
						task();
					}
				}));				
			}	
		}
	
		template<class Function, class... Args>
		std::future<typename std::result_of<Function(Args...)>::type>
			ThreadPool::add(Function&& fcn, Args&&... args)
		{
			typedef typename std::result_of<Function(Args...)>::type return_type;
			typedef std::packaged_task<return_type()> task;
	
			auto t = std::make_shared<task>(std::bind(std::forward<Function>(fcn), std::forward<Args>(args)...));
			auto ret = t->get_future();
			{
				std::lock_guard<std::mutex> lg(this->_mtx);
				if (this->_stop.load(std::memory_order_acquire)) {
					throw std::runtime_error("thread pool has stopped");
				}
				this->_tasks.emplace([t]{(*t)(); });
			}
			this->_cond.notify_one();
			return ret;
		}
}

#endif
