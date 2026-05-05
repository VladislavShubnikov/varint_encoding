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

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <vector>
#include <cstdint>

namespace compression
{

class Compressor
{
public:
    Compressor() = default;

    /**
     * Compress 2D points vector into internal storage
     * @param vec: vector of points in pairs: first-x, second-y
     */
    void pack(const std::vector<std::pair<int, int>>& vec);

    /**
     * This function can be useful to
     * calculate compression ratio
     * @return Internal storage size in bytes
     */
    int getStorageSizeBytes() const;

    /**
     * Unpack compressed storage into normal 2D points vector
     * @details: This method is not const
     * due to possible reallocation in deltas_, mergedXY_
     * @return: vector of 2D points, each point is pair: x-first, y-second
     */
    std::vector<std::pair<int, int>> unpack();

private:
    //! number of points (used in unpack to speedup memory allocations)
    std::size_t numPoints_{0};
    //! packed variable-length bit codes
    std::vector<uint8_t> codes_;

    //! delta for merged xy coordinates of points (used in pack only, can be optimized)
    std::vector<uint32_t> deltas_;
    //! merged xy coordinates of points (used in pack only, can be optimized)
    std::vector<uint32_t> mergedXY_;
  
}; // Compressor

} // namespace compression

#endif // COMPRESSOR_H
