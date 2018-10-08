using namespace std;

#include "SlowClosingMockIndexInputWrapper.h"

namespace org::apache::lucene::store
{
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

SlowClosingMockIndexInputWrapper::SlowClosingMockIndexInputWrapper(
    shared_ptr<MockDirectoryWrapper> dir, const wstring &name,
    shared_ptr<IndexInput> delegate_)
    : MockIndexInputWrapper(dir, name, delegate_, nullptr)
{
}

SlowClosingMockIndexInputWrapper::~SlowClosingMockIndexInputWrapper()
{
  try {
    delay(50);
  } catch (const InterruptedException &ie) {
    throw make_shared<ThreadInterruptedException>(ie);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // C++ NOTE: There is no explicit call to the base class destructor in C++:
    //        super.close();
  }
}
} // namespace org::apache::lucene::store