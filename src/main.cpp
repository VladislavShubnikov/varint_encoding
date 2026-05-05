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

#include <random>
#include <vector>
#include <utility>
#include <iomanip>
#include <algorithm>
#include <iostream>

#include "duration_meter.h"
#include "compressor.h"

namespace
{

void assertMsg(bool condition, const char* message)
{
    if (!condition)
    {
        std::cout << "!Assertion!: " << message << "\n";
    }
}    

void test()
{
    const std::vector<std::pair<int, int>> vecIn{
        {10, 4}, {8, 5}, {11, 4}, {3, 8}, {29, 8}, 
        {22, 3}, {35, 15}, {123, 8}, {2, 287}, {34, 2}
    };

    compression::Compressor compressor;
    compressor.pack(vecIn);

    const std::vector<std::pair<int, int>> vecOut = compressor.unpack();
    assertMsg(vecOut.size() == vecIn.size(), "different given/expected size");

    // points in expected and given vectors can be in different order
    for (const auto& p: vecOut)
    {
        const auto it = std::find(vecIn.begin(), vecIn.end(), p);
        assertMsg(it != vecIn.end(), "point in given array is not in expected");
    }

    std::cout << "minimal test completed. " << "\n";
}   

void testRand()
{
    constexpr int kNumThousandPoints{12};
    constexpr int kNumPoints{1000 * kNumThousandPoints};
    std::random_device randDevice; 
    std::mt19937 gen(randDevice());
    std::vector<std::pair<int, int>> vecIn;

    for (int i = 0; i < kNumPoints; i++)
    {
        const auto x = 0 + static_cast<int>(gen() & 65535);
        const auto y = 0 + static_cast<int>(gen() & 65535);
        vecIn.emplace_back(x, y);
    }

    compression::Compressor compressor;

    const timer::DurationMeter meterPack;
    compressor.pack(vecIn);
    const double timePackMs = meterPack.getMilliseconds();

    const timer::DurationMeter meterUnpack;
    const std::vector<std::pair<int, int>> vecOut = compressor.unpack();
    const double timeUnpackMs = meterUnpack.getMilliseconds();

    assertMsg(vecOut.size() == vecIn.size(), "different given/expected size");

    // points in expected and given vectors can be in different order
    for (const auto& p: vecOut)
    {
        const auto it = std::find(vecIn.begin(), vecIn.end(), p);
        assertMsg(it != vecIn.end(), "point in given array is not in expected");
    }

    // performance report
    constexpr int kPrinitDoublePrecision{3};
    const double packTimePerMillion = 1000.0 * timePackMs / kNumThousandPoints;
    std::cout << "pack time for million points =  " << 
    std::fixed << std::setprecision(kPrinitDoublePrecision) << 
    packTimePerMillion << " milliseconds" << "\n";

    const double unpackTimePerMillion = 1000.0 * timeUnpackMs / kNumThousandPoints;
    std::cout << "unpack time for million points =  " << 
    std::fixed << std::setprecision(kPrinitDoublePrecision) << 
    unpackTimePerMillion << " milliseconds" << "\n";

    // data compression report
    const int storageSize = compressor.getStorageSizeBytes();
    const int rawDataSize = static_cast<int>(vecIn.size() * 2 * 4);

    const double ratio = 
        100.0 * 
        static_cast<double>(storageSize) / 
        static_cast<double>(rawDataSize);

    std::cout << "compression ratio =  " << 
    std::fixed << std::setprecision(1) << ratio << "%" << "\n";

    std::cout << "random test completed. " << "\n";
}

} // namespace

int main() // NOLINT(bugprone-exception-escape)
{
    test();
    testRand();
    return 0;
}
