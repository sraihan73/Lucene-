using namespace std;

#include "SlowOpeningMockIndexInputWrapper.h"

namespace org::apache::lucene::store
{
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

SlowOpeningMockIndexInputWrapper::SlowOpeningMockIndexInputWrapper(
    shared_ptr<MockDirectoryWrapper> dir, const wstring &name,
    shared_ptr<IndexInput> delegate_) 
    : MockIndexInputWrapper(dir, name, delegate_, nullptr)
{
  try {
    delay(50);
  } catch (const InterruptedException &ie) {
    try {
      // C++ NOTE: There is no explicit call to the base class destructor in
      // C++:
      //          super.close();
    } catch (const runtime_error &ignore) {
    } // we didnt open successfully
    throw make_shared<ThreadInterruptedException>(ie);
  }
}
} // namespace org::apache::lucene::store