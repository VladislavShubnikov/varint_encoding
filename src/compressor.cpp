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
// 2D point cloud simple compressor
// Based on multidimension bit merging + 
// deltas + variable bit length encoding

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <vector>

#include "compressor.h"

void compression::Compressor::pack(const std::vector<std::pair<int, int>>& vec)
{
    // prepare containers capacity to minimize memory reallocations
    numPoints_ = vec.size();
    mergedXY_.resize(numPoints_);
    deltas_.resize(numPoints_);

    // hope that compression ratio will be not more 50%
    codes_.reserve(numPoints_ / 2);

    // 4 bytes in input 32 bits value
    constexpr uint32_t k4{4};
    // output data is byte
    // split large integer into 7+7+7+... parts
    constexpr uint32_t k7{7};
    // output data is byte with 8 bits
    constexpr uint32_t k8{8};
    // maximum value to decide: should continue value split or not
    constexpr uint32_t k127{(1 << k7) - 1};
    // bit mask to extract bytes from 32 bit integer
    constexpr uint32_t k255{(1 << k8) - 1};

    int i{0};

    for (const auto& p: vec)
    {
        auto x = static_cast<uint32_t>(p.first);
        auto y = static_cast<uint32_t>(p.second);
        uint32_t out{0};
        uint32_t pos{0};

        while ((x > 0) || (y > 0))
        {
            const uint32_t bitX = (x & 1);
            x >>= 1;
            const uint32_t bitY = (y & 1);
            y >>= 1;
            out = out | (bitX << pos) | (bitY << (pos + 1));
            pos += 2;
        }

        mergedXY_[i++] = out;
    }

    // packed points will not save the original order:
    // they will be sorted based on (x,y)->code value
    // Sorting is required here to build a delta array
    // with relatively small values (will be compressed better).
    // Also sorting is important to have a non-decreasing sequence
    // of numbers in order to store delta values and, as consequence,
    // significantly decrease bits dimension.
    std::sort(mergedXY_.begin(), mergedXY_.end());

    // The first element is code itself, 
    // others will store actual deltas
    deltas_[0] = mergedXY_[0];

    for (std::size_t i = 1; i < numPoints_; i++)
    {
        const uint32_t delta{mergedXY_[i] - mergedXY_[i - 1]};
        deltas_[i] = delta;
    }

    // first value put as 4 bytes
    uint32_t val = deltas_[0];

    for (uint32_t k = 0; k < k4; k++)
    {
        const uint8_t byte = val & k255;
        val >>= k8;
        codes_.push_back(byte);
    }

    uint8_t byte{0};

    // encode deltas
    for (std::size_t i = 1; i < numPoints_; i++)
    {
        val = deltas_[i];

        if (val == 0)
        {
            byte = 1;
            codes_.push_back(byte);
            continue;
        }

        while (val > 0)
        {
            if (val <= k127)
            {
                // first bit is 1
                byte = 1;
                // next 7 bits is val
                byte |= static_cast<uint8_t>(val << 1);
                codes_.push_back(byte);
            }
            else
            {
                byte = 0;
                byte |= static_cast<uint8_t>((val & k127) << 1);
                codes_.push_back(byte);

            }
            val >>= k7;
        }
    }
} // pack


int compression::Compressor::getStorageSizeBytes() const
{
    return static_cast<int>(codes_.size());
}


std::vector<std::pair<int, int>> compression::Compressor::unpack()
{
    // output points vector
    std::vector<std::pair<int, int>> out;
    // minimize memory reallocations
    out.reserve(numPoints_);

    // function to convert merged bits to pair (x,y)
    const auto mergedToXY = [&out](uint32_t merged){
        uint32_t x{0};
        uint32_t y{0};
        uint32_t pos{0};

        while(merged > 0)
        {
            const uint32_t bitX = merged & 1;
            merged >>= 1;
            const uint32_t bitY = merged & 1;
            merged >>= 1;
            x = x | (bitX << pos);
            y = y | (bitY << pos);
            pos++;
        }
        out.emplace_back(x,y);
    };

    int i{0};
    uint32_t val{0};
    int shift{0};
    uint32_t prevVal{0};
    constexpr int k4{4};
    constexpr int k7{7};
    constexpr int k8{8};

    for (const uint8_t code: codes_)
    {
        if (i < k4)
        {
            val |= (static_cast<uint32_t>(code) << (i * k8));
            i++;

            if (i == k4)
            {
                mergedToXY(val);
                prevVal = val;

                val = 0;
                shift = 0;
            }
            continue;
        }
        
        if ((code & 1) == 1)
        {
            // 1st bit: value in [0..127]
            const uint32_t bits = static_cast<uint32_t>(code) >> 1;
            val |= bits << shift;

            // here "val" is delta to the previous value
            const uint32_t valNew = prevVal + val;
            prevVal = valNew;
            mergedToXY(valNew);

            val = 0;
            shift = 0;
        }
        else
        {
            // accumulate val
            const auto bits = static_cast<uint32_t>(code) >> 1;
            val |= bits << shift;
            shift += k7;
        }

        i++;
    }

    return out;
} // unpack
