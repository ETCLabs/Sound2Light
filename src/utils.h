// Copyright (c) 2016 Electronic Theatre Controls, Inc., http://www.etcconnect.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef UTILS_H
#define UTILS_H

#include <QtMath>
#include <chrono>

// This file includes some often used utility functions.


// Limits / saturates a value with min and max bounds:
template<typename T1, typename T2, typename T3>
T2 limit(T1 min, T2 value, T3 max) {
	return qMax(T2(min), qMin(value, T2(max)));
}


namespace HighResTime {  // -------------------

typedef std::chrono::time_point<std::chrono::system_clock> time_point_t;

inline time_point_t now() {
    return std::chrono::system_clock::now();
}

inline double elapsedSecSince(time_point_t start) {
    time_point_t now = HighResTime::now();
    std::chrono::duration<double> elapsed_seconds_duration = now - start;
    double elapsedSeconds = elapsed_seconds_duration.count();
    return elapsedSeconds;
}

inline double diff(time_point_t end, time_point_t start) {
    std::chrono::duration<double> elapsed_seconds_duration = end - start;
    return elapsed_seconds_duration.count();
}

inline double getElapsedSecAndUpdate(time_point_t& lastTime) {
    time_point_t now = HighResTime::now();
    double elapsedSeconds = diff(now, lastTime);
    lastTime = now;
    return elapsedSeconds;
}

}  // end namespace HighResTime -----------------


#endif // UTILS_H
