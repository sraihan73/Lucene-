using namespace std;

#include "CloseableDirectory.h"

namespace org::apache::lucene::util
{
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using org::junit::Assert;

CloseableDirectory::CloseableDirectory(
    shared_ptr<BaseDirectoryWrapper> dir,
    shared_ptr<TestRuleMarkFailure> failureMarker)
    : dir(dir), failureMarker(failureMarker)
{
}

CloseableDirectory::~CloseableDirectory()
{
  // We only attempt to check open/closed state if there were no other test
  // failures.
  try {
    if (failureMarker->wasSuccessful() && dir->isOpen()) {
      Assert::fail(L"Directory not closed: " + dir);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // TODO: perform real close of the delegate: LUCENE-4058
    // dir.close();
  }
}
} // namespace org::apache::lucene::util