// The MIT License (MIT)
//
// Copyright (c) 2026 Vladislav Shubnikov
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
//
// Measure time for given code range
// between constructor and getMilliseconds function call


#include <iostream>
#include <chrono>
#include <cstdint>

#include "duration_meter.h"


timer::DurationMeter::DurationMeter()
:startTime_(std::chrono::steady_clock::now())
{
}

double timer::DurationMeter::getMilliseconds() const
{
    try 
    {
        const auto endTime = std::chrono::steady_clock::now();
        const auto dur = endTime - startTime_;
        const int64_t dtMicro = 
            std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
        const double dtMilli = static_cast<double>(dtMicro) / 1000.0;
        return dtMilli;
    } catch (...)
    {
        std::cout << "Error in ~LogDuration" << "\n";
    }
    return 0.0;
}

