#pragma once

#include <climits>
#include <chrono>
#include <functional>
#include <vector>
#include <random>
#include <thread>
#include <array>
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
}
