using namespace std;

#include "ReaderSlice.h"

namespace org::apache::lucene::index
{

std::deque<std::shared_ptr<ReaderSlice>> const ReaderSlice::EMPTY_ARRAY =
    std::deque<std::shared_ptr<ReaderSlice>>(0);

ReaderSlice::ReaderSlice(int start, int length, int readerIndex)
    : start(start), length(length), readerIndex(readerIndex)
{
}

wstring ReaderSlice::toString()
{
  return L"slice start=" + to_wstring(start) + L" length=" +
         to_wstring(length) + L" readerIndex=" + to_wstring(readerIndex);
}
} // namespace org::apache::lucene::index