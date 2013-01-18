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

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>
using namespace std;

template <unsigned int N>
struct OutputStream {
public:
    OutputStream(FILE *fp)
        : m_fp(fp)
        , m_n(0)
    {
    }

    ~OutputStream()
    {
        flush();
        fclose(m_fp);
    }

    inline OutputStream<N> &operator<<(unsigned int i)
    {
        m_buffer[m_n++] = i;

        if (m_n == N)
            flush();

        return *this;
    }

    inline void flush()
    {
        static const int bufferSize = N * 16;
        char buf[bufferSize];

        char *p = buf;
        for (int i = 0; i < m_n; ++i) {
            unsigned int j = m_buffer[i];

            char tmp[16];
            int k = 15;
            while (j) {
                tmp[k--] = j % 10;
                j /= 10;
            }

            for (++k; k < 16; ++k)
                *p++ = '0' + tmp[k];

            *p++ = '\n';
        }

        if (m_n) {
            fwrite(buf, 1, p - buf, m_fp);
            m_n = 0;
        }
    }


private:
    FILE *m_fp;
    unsigned int m_n;
    unsigned int m_buffer[N];
};

template <>
struct OutputStream<0>;

class BitAccess
{
public:
    BitAccess(unsigned int *data)
        : m_data(data)
    {
    }

    inline void set(unsigned int i)
    {
        const unsigned int index = i >> 5;
        const unsigned int bit = i & 0x1f;

        m_data[index] |= 1 << bit;
    }

    inline unsigned int get(unsigned int i) const
    {
        const unsigned int index = i >> 5;
        const unsigned int bit = i & 0x1f;

        return m_data[index] & (1 << bit);
    }

private:
    unsigned int *m_data;
};

int main(int argc, char **argv)
{
    unsigned int candidates;
    if (!(argc > 1 && sscanf(argv[1], "%u", &candidates)))
        candidates = 10000000;

    printf("Storing primes up to %u in out.txt\n", candidates);

    const char *filename = "out.txt";
    FILE *out = fopen(filename, "w");
    if (!out) {
        printf("Couldn't open file %s\n", filename);
        return -1;
    }

    vector<unsigned int> data(std::max(1U, ((candidates + 1) >> 6)));
    BitAccess notPrime(&*data.begin());
    notPrime.set(0);

    OutputStream<1024> ostream(out);

    if (candidates >= 2)
        ostream << 2;

    const unsigned int next = static_cast<unsigned int>(sqrt(candidates)) | 1;
    for (unsigned int i = 3; i < next; i += 2) {
        const unsigned int half = i >> 1;
        if (!notPrime.get(half)) {
            const unsigned int last = candidates >> 1;
            for (unsigned int j = i + half; j <= last; j += i)
                notPrime.set(j);

            ostream << i;
        }
    }

    for (unsigned int i = next; i <= candidates; i += 2)
        if (!notPrime.get(i >> 1))
            ostream << i;

    printf("Done!\n");
    return 0;
}
