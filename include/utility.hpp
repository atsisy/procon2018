#pragma once

#include <climits>
#include <chrono>
#include <functional>
#include <vector>
#include <random>
#include <thread>

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
                unsigned random()
                {
                        u32 t;
                        t = x ^ (x << 11);
                        x = y;
                        y = z;
                        z = w;
                        return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
                }
                
        public:
                unsigned operator()(){ return random(); }
                xor128()
                {
                        std::random_device rd;x
                        w = rd();
                }
                xor128(u32 s){ w = s; }  // 与えられたシードで初期化
        };

}
