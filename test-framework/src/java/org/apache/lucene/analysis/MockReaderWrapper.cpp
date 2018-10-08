using namespace std;

#include "MockReaderWrapper.h"

namespace org::apache::lucene::analysis
{
using TestUtil = org::apache::lucene::util::TestUtil;

MockReaderWrapper::MockReaderWrapper(shared_ptr<Random> random,
                                     shared_ptr<Reader> in_)
    : in_(in_), random(random)
{
}

void MockReaderWrapper::throwExcAfterChar(int charUpto)
{
  excAtChar = charUpto;
  // You should only call this on init!:
  assert(readSoFar == 0);
}

void MockReaderWrapper::throwExcNext() { throwExcNext_ = true; }

MockReaderWrapper::~MockReaderWrapper() { in_->close(); }

int MockReaderWrapper::read(std::deque<wchar_t> &cbuf, int off,
                            int len) 
{
  if (throwExcNext_ || (excAtChar != -1 && readSoFar >= excAtChar)) {
    throw runtime_error(L"fake exception now!");
  }
  constexpr int read;
  constexpr int realLen;
  if (len == 1) {
    realLen = 1;
  } else {
    // Spoon-feed: intentionally maybe return less than
    // the consumer asked for
    realLen = TestUtil::nextInt(random, 1, len);
  }
  if (excAtChar != -1) {
    constexpr int left = excAtChar - readSoFar;
    assert(left != 0);
    read = in_->read(cbuf, off, min(realLen, left));
    assert(read != -1);
    readSoFar += read;
  } else {
    read = in_->read(cbuf, off, realLen);
  }
  return read;
}

bool MockReaderWrapper::markSupported() { return false; }

bool MockReaderWrapper::ready() { return false; }

bool MockReaderWrapper::isMyEvilException(runtime_error t)
{
  return (dynamic_cast<runtime_error>(t) != nullptr) &&
         L"fake exception now!" == t.what();
}
} // namespace org::apache::lucene::analysis