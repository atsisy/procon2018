#pragma once

#include <climits>
#include <chrono>
#include <functional>
#include <vector>
#include <random>
#include <thread>
#include <array>
#include <deque>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <cassert>

#include "types.hpp"

namespace util {

        /*
         * 時間の単位を再定義
         */
        using t_s  = std::chrono::seconds;
        using t_ms = std::chrono::milliseconds;
        using t_us = std::chrono::microseconds;

        /*
         * measure_processing_time関数
         * 渡された関数の処理時間を計測する関数
         * 引数
         * function: 計測する関数
         * 返り値
         * 指定単位の処理時間
         */
        template <typename Unit>
        double measure_processing_time(std::function<void(void)> function)
        {
                std::chrono::system_clock::time_point  start, end;
                start = std::chrono::system_clock::now();

                function();
        
                end = std::chrono::system_clock::now();
                return std::chrono::duration_cast<Unit>(end-start).count();

        }

        template <class... Args>
        std::function<void(void)> run_single_thread(std::function<void(Args&&...)> process, Args&&... args)
        {
                return [&](){
                               std::mutex mtx_;
                               {
                                       mtx_.lock();
                                       process(args...);
                                       mtx_.unlock();
                               }
                       };
        }


        class xor128 {
        private:
                u32 x = 123456789, y = 362436069u, z = 521288629, w;
                u32 random()
                        {
                                u32 t;
                                t = x ^ (x << 11);
                                x = y;
                                y = z;
                                z = w;
                                return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
                        }
                
        public:
                u32 operator()(){ return random(); }
                xor128()
                        {
                                std::random_device rd;
                                w = rd();
                        }
                xor128(u32 s){ w = s; }  // 与えられたシードで初期化
        };

        template <class T>
        class queue {
        private:
                std::array<T, 8192> buffer;
                u64 head;
                u64 tail;
        public:
                queue()
                        {
                                head = 0;
                                tail = 0;
                        }

                void push_back(T obj)
                        {
                                buffer[tail] = obj;
                                tail++;
                        }

                void clear()
                        {
                                head = 0;
                                tail = 0;  
                        }

                size_t size()
                        {
                                return tail - head;
                        }

                T front()
                        {
                                return buffer[head];
                        }

                void pop_front()
                        {
                                head++;
                        }
        };

        template <typename T>
        class Queue2{
        public:
                Queue2(int size)
                        : size_(size)
                        {}
                bool put(T&& data) {
                        if (size_ <= deque_.size()) {
                                return false;
                        }
                        deque_.emplace_back(std::move(data));
                        return true;
                }
                bool put(const T& data) {
                        if (size_ <= deque_.size()) {
                                return false;
                        }
                        deque_.emplace_back(data);
                        return true;
                }
                bool get(T& data) {
                        if (deque_.empty()) {
                                return false;
                        }
                        data = std::move(deque_.front());
                        deque_.pop_front();
                        return true;
                }
                bool empty() const {
                        return deque_.empty();
                }
        private:
                int size_;
                std::deque<T> deque_;
        };

        class Runnable{
        public:
                virtual void run() = 0;
        };

        class ThreadPool{
        public:
                ThreadPool(int threadCount, int queueSize)
                        : isTerminationRequested_(false)
                        , queue_(queueSize)
                        {
                                for (int n = 0; n < threadCount; n++) {
                                        threads_.emplace_back(std::thread(main_));
                                }
                        }
                ~ThreadPool() {
                        {
                                std::unique_lock<std::mutex> ul(mutex_);
                                isTerminationRequested_ = true;
                        }
                        cv_.notify_all();
                        const int size = threads_.size();
                        for (int n = 0; n < size; n++) {
                                threads_.at(n).join();
                        }
                }
                bool add(std::shared_ptr<Runnable> &&runnable) {
                        {
                                std::unique_lock<std::mutex> ul(mutex_);
                                if (!queue_.put(std::move(runnable))) { return false; }
                        }
                        cv_.notify_all();
                        return true;
                }
                bool add(const std::shared_ptr<Runnable> &runnable) {
                        {
                                std::unique_lock<std::mutex> ul(mutex_);
                                if (!queue_.put(runnable)) { return false; }
                        }
                        cv_.notify_all();
                        return true;
                }
        private:
                std::function<void()> main_ = [this]()
                                                      {
                                                              while (1) {
                                                                      std::shared_ptr<Runnable> runnable;
                                                                      {
                                                                              std::unique_lock<std::mutex> ul(mutex_);
                                                                              while (queue_.empty()) {
                                                                                      if (isTerminationRequested_) { return; }
                                                                                      cv_.wait(ul);
                                                                              }
                                                                              const bool result = queue_.get(runnable);
                                                                              assert(result);
                                                                      }
                                                                      runnable->run();
                                                              }
                                                      };
                bool isTerminationRequested_;
                Queue2<std::shared_ptr<Runnable>> queue_;
                std::mutex mutex_;
                std::condition_variable cv_;
                std::vector<std::thread> threads_;
        };        
}
