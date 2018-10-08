using namespace std;

#include "ParallelPostingsArray.h"

namespace org::apache::lucene::index
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

ParallelPostingsArray::ParallelPostingsArray(int const size)
    : size(size), textStarts(std::deque<int>(size)),
      intStarts(std::deque<int>(size)), byteStarts(std::deque<int>(size))
{
}

int ParallelPostingsArray::bytesPerPosting() { return BYTES_PER_POSTING; }

shared_ptr<ParallelPostingsArray> ParallelPostingsArray::newInstance(int size)
{
  return make_shared<ParallelPostingsArray>(size);
}

shared_ptr<ParallelPostingsArray> ParallelPostingsArray::grow()
{
  int newSize = ArrayUtil::oversize(size + 1, bytesPerPosting());
  shared_ptr<ParallelPostingsArray> newArray = newInstance(newSize);
  copyTo(newArray, size);
  return newArray;
}

void ParallelPostingsArray::copyTo(shared_ptr<ParallelPostingsArray> toArray,
                                   int numToCopy)
{
  System::arraycopy(textStarts, 0, toArray->textStarts, 0, numToCopy);
  System::arraycopy(intStarts, 0, toArray->intStarts, 0, numToCopy);
  System::arraycopy(byteStarts, 0, toArray->byteStarts, 0, numToCopy);
}
} // namespace org::apache::lucene::index