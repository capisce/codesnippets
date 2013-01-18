/*
   Copyright (c) 2013 Samuel Rødal

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/**
 * Computes all the permutations of bit strings of a
 * given length containing a given number of 1-bits.
 **/

#include <algorithm>
using std::max;
using std::min;

#include <iostream>
using std::cout;

#include <string>
using std::string;

#include <vector>
using std::vector;

// uncomment if you want to see the permutations
//#define DISPLAY_PERMUTATIONS

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

class PermutationFinder
{
public:
    PermutationFinder(int string_length, int bits_on);

#ifdef DISPLAY_PERMUTATIONS
    void display(uint64 x) const;
    void display(uint32 high, uint32 low) const;
#endif

    void displayExpectedPermutationCount();
    void computePermutations() const;

private:
    int m_string_length;
    int m_bits_on;

#ifdef DISPLAY_PERMUTATIONS
    mutable string m_str;
#endif
};

PermutationFinder::PermutationFinder(int string_length, int bits_on)
    : m_string_length(string_length)
    , m_bits_on(bits_on)
{
#ifdef DISPLAY_PERMUTATIONS
    m_str.resize(string_length);
#endif
}

// jump[]: map from uint8 to next uint8 with same bit count
uint16 jump[256];

// jump[][0]: map from uint8 to next uint8 with higher bit count
// jump[][1]: map from uint8 to next uint8 with lower bit count
uint16 jump_out[256][2];

// precomputed uint8 bitcounts
uint8 count[256];

inline int bitcount_naive(uint32 x)
{
    int c = 0;
    while (x) {
        while (!(x & 1))
            x >>= 1;
        ++c;
        x >>= 1;
    }
    return c;
}

inline int bitcount(uint32 x)
{
    return count[(x)       & 0xff] +
           count[(x >> 8)  & 0xff] +
           count[(x >> 16) & 0xff] +
           count[(x >> 24) & 0xff];
}

void precompute()
{
    for (int i = 0; i < 256; ++i) {
        count[i] = bitcount_naive(i);

        if (i == 0) {
            jump[i] = 256;
            jump_out[i][0] = 1;
            jump_out[i][1] = 256;
        } else {
            int j = 1;
            for (; j < 512; ++j) {
                if (bitcount_naive(i + j) == count[i])
                    break;
            }
            jump[i] = j;
            j = 1;
            for (; j < 512; ++j) {
                if (bitcount_naive(i + j) > count[i])
                    break;
            }
            jump_out[i][0] = j;
            j = 1;
            for (; j < 256; ++j) {
                if (bitcount_naive(i + j) < count[i])
                    break;
            }
            jump_out[i][1] = j;
        }
    }
}

#ifdef DISPLAY_PERMUTATIONS
inline void PermutationFinder::display(uint64 x) const
{
    for (int i = 0; i < m_string_length; ++i) {
        m_str[m_string_length-1-i] = '0' + (x & 1);
        x >>= 1;
    }

    cout << m_str << '\n';
}

inline void PermutationFinder::display(uint32 high, uint32 low) const
{
    display((uint64(high) << 32) + low);
}
#endif

// computes the expected permutation count
void PermutationFinder::displayExpectedPermutationCount()
{
    vector<uint64> tbl((m_string_length+1)*(m_string_length+1));

#define index(i, j) (((i) * (m_string_length + 1)) + (j))
    // dynamic programming
    tbl[index(0, 0)] = 1;
    for (int x = 1; x <= m_string_length; ++x) {
        tbl[index(x, 0)] = 1;
        for (int n = 1; n <= x; ++n) {
            tbl[index(x, n)] = tbl[index(x-1, n-1)];
            if (n < x)
                tbl[index(x, n)] += tbl[index(x-1, n)];
        }
    }

    cout << "Expected permutation count of " << m_string_length
         << " length string with " << m_bits_on << " 1 bits: "
         << tbl[index(m_string_length, m_bits_on)] << '\n';
#undef index
}

// computes all the permutations in sorted order
void PermutationFinder::computePermutations() const
{
    uint64 c = 0;

    const uint32 end_high = 1 << max(m_string_length - 31, 0);
    const uint32 end_low = 1 << min(m_string_length, 31);

    const uint32 mask_bits = ~min(end_low-1, uint32(0xff));

    for (uint32 high = 0; high < end_high; ++high) {
        const int high_count = bitcount(high);
        if (high_count == m_bits_on) {
            ++c;
#ifdef DISPLAY_PERMUTATIONS
            display(high, 0);
#endif
        } else if (high_count < m_bits_on) {
            // number of bits that should be on of the lower 32 bits
            const int remaining = m_bits_on - high_count;
            for (uint32 low = 1; low < end_low;) {
                const int b = bitcount(low);
                if (b == remaining) {
                    // now loop through all permutations of lower 8 bits with
                    // the same bitcount until any of the higher bits change
                    const uint32 mask = low & mask_bits;
                    do {
                        ++c;
#ifdef DISPLAY_PERMUTATIONS
                        display(high, low);
#endif
                        // go to next uint32 with same bitcount in lower 8 bits
                        low += jump[low & 0xff];
                    } while ((low & mask_bits) == mask);
                } else {
                    // if current bit count is higher than the one we want
                    // go to next uint32 with lower bit count in lower 8 bits
                    // else
                    // go to next uint32 with higher bit count in lower 8 bits
                    low += jump_out[low & 0xff][b > remaining];
                }
            }
        }
    }

    cout << "Computed " << c << " permutations of " << m_string_length
         << " length bit strings with " << m_bits_on << " 1 bits\n";
}

int main()
{
    PermutationFinder finder(35, 15);
    precompute();
    finder.displayExpectedPermutationCount();
    finder.computePermutations();

    return 0;
}
