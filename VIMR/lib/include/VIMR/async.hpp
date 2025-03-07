#pragma once

#include <mutex>
#include <chrono>
#include <thread>
#include <string>
#include <functional>
#include <condition_variable>
#include <utility>
#include <iostream>

namespace VIMR
{
	/*
	 * A thread-safe which supports producer-consumer pipelines which can either aim to minimise latency or minimise data loss
	 */
	template<class T>
	class RingBuffer
	{
	 protected:
			std::mutex cond_signal_mutex;
			std::condition_variable cond_full;
			std::condition_variable cond_empty;
			bool is_released = false;
			size_t q_size;
			size_t head_idx = 0;
			size_t tail_idx = 0;
			T* buffer;
	 public:
			size_t size() const
			{
				return q_size;
			}
			size_t num_used() const
			{
				return head_idx - tail_idx;
			}
			/*
			 * _size >= 3
			 */
			explicit RingBuffer(size_t _size)
			{
				release = [this]()
				{
					is_released = true;
					cond_full.notify_one();
					cond_empty.notify_one();
				};
				q_size = _size;
				if (q_size < 3) throw std::exception();
				buffer = new T[q_size]{};
			}
			/*
			 * Make sure to call release() and to join() any threads using the buffer before destruction.
			 */
			~RingBuffer()
			{
				release();

				//delete[] buffer;
			}

			/*
			 * Advances the head to next un-occupied element (use current_head() to access the head element)
			 *
			 * If there are no un-occupied elements (i.e. the buffer is full), then this call will block until
			 * the buffer is no longer full (i.e. another thread calls advance_tail()) or until release() is called.
			 */
			void advance_head()
			{
				std::unique_lock<std::mutex> cond_sig_lock(cond_signal_mutex);
				cond_full.wait(cond_sig_lock, [this]()
				{ return !this->head_advance_blocked() || this->is_released; });
				head_idx++;
				cond_empty.notify_one();
			}

			/*
			 * Returns true and advances the head if the buffer is not full.
			 * Returns false and does not advance the head if the buffer is full.
			 * Never blocks.
			 */
			bool try_advance_head()
			{
				if (this->head_advance_blocked()) return false;
				advance_head();
				return true;
			}

			/*
			 * Returns the pointer to the current head element.
			 * Always check that released() returns false before using the result of this.
			 */
			T* current_head()
			{
				return &(buffer[head_idx % q_size]);
			}

			/*
			 * Returns the pointer to the current tail element.
			 * Always check that released() returns false before using the result of this.
			 */
			T* current_tail()
			{
				return &(buffer[tail_idx % q_size]);
			}

			/*
			 * Advances the tail to the next occupied element, and returns a pointer to the new tail element.
			 * If there are no occupied elements available (i..e the buffer is empty), then this call will block until
			 * the buffer is no longer empty (i.e. another thread advances the buffer head, or untill release() is called.
			 *
			 * Always check that released() returns false before using the result of this.
			 */
			T* advance_tail()
			{
				std::unique_lock<std::mutex> cond_sig_lock(cond_signal_mutex);
				cond_empty.wait(cond_sig_lock, [this]()
				{ return !this->tail_advance_blocked() || this->is_released; });
				T* x = &(buffer[tail_idx++ % q_size]);
				cond_full.notify_one();
				return x;
			}

			/*
			 * Same as advance_tail(), but will only block for _timeout_ms if the buffer is empty.
			 * If the buffer is still empty after _timeout_ms then this will return nullptr
			 * If another thread advances the buffer head then this will return immediately (even if _timeout_ms has not elapsed)
			 */
			T* advance_tail(unsigned long _timeout_ms)
			{
				std::unique_lock<std::mutex> cond_sig_lock(cond_signal_mutex);
				if (!cond_empty.wait_for(cond_sig_lock, std::chrono::milliseconds(_timeout_ms), [this]()
				{ return !this->tail_advance_blocked() || this->is_released; }))
					return nullptr;
				T* x = &(buffer[tail_idx++ % q_size]);
				cond_full.notify_one();
				return x;
			}

			/*
			 * If the next tail element is occupied, return a pointer to that element otherwise return nullptr.
			 * Does not advance the tail of the buffer.
			 * Will never block
			 */
			T* peek_tail()
			{
				if (tail_advance_blocked()) return nullptr;
				return &(buffer[(tail_idx + 1) % q_size]);
			}

			bool head_advance_blocked() const
			{
				/*
				 * Current tail pointer is at ((tail_idx-1) % q_size) rather than at (tail_idx % q_size)
				 * So need an extra element between tail_idx and head_idx, which here is added as a '-1'
				 * on the RHS for semantic correcntess with the above line.
				 */
				return (head_idx + 1) >= (q_size + tail_idx - 1);
			}
			bool tail_advance_blocked() const
			{
				return tail_idx >= head_idx;
			}
			bool released()
			{
				return is_released;
			}
			std::function<void(void)> release;

			void reset()
			{
				head_idx = 0;
				tail_idx = 0;
			}
	};

	/*
	 * An extension of RingBuffer which manages a consumer thread
	 */
	template<class T>
	class BufferProcessor : public RingBuffer<T>
	{
	 public:
			typedef std::function<void(T*)> ProcessFunction;

			BufferProcessor(size_t _size, ProcessFunction _process_function) : RingBuffer<T>(_size)
			{
				this->release = [this]()
				{
					this->is_released = true;
					this->cond_full.notify_one();
					this->cond_empty.notify_one();
					if (process_thread.joinable())
						process_thread.join();
				};
				process_function = _process_function;
				process_thread = std::thread([this]()
				{
					this->invoke_callback();
				});
			}
			~BufferProcessor()
			{
				this->is_released = true;
				this->cond_full.notify_one();
				this->cond_empty.notify_one();
				if (process_thread.joinable())
					process_thread.join();
//				delete[] buffer;
			}
	 private:
			ProcessFunction process_function;
			std::thread process_thread;
			void invoke_callback()
			{
				while (!this->is_released)
				{
					T* current = this->advance_tail();
					if (this->is_released) break;

					process_function(current);
				}
			}
	};

	/*
	 * Non-busy execution blocking
	 */
	class Waiter
	{
	 public:
			void wait()
			{
				std::unique_lock<std::mutex> cv_lock(cv_mut);
				cv.wait(cv_lock);
			}
			void signal()
			{
				cv.notify_all();
			}
	 private:
			std::condition_variable cv;
			std::mutex cv_mut;
	};
}
