using namespace std;

#include "RemoveUponClose.h"

namespace org::apache::lucene::util
{

// C++ TODO: There is no native C++ equivalent to 'toString':
RemoveUponClose::RemoveUponClose(shared_ptr<Path> path,
                                 shared_ptr<TestRuleMarkFailure> failureMarker)
    : path(path), failureMarker(failureMarker), creationStack(b->toString())
{

  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  for (shared_ptr<StackTraceElement> e :
       Thread::currentThread().getStackTrace()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    b->append(L'\t')->append(e->toString())->append(L'\n');
  }
}

RemoveUponClose::~RemoveUponClose()
{
  // only if there were no other test failures.
  if (failureMarker->wasSuccessful()) {
    if (Files::exists(path)) {
      try {
        IOUtils::rm({path});
      } catch (const IOException &e) {
        throw make_shared<IOException>(
            L"Could not remove temporary location '" + path->toAbsolutePath() +
                L"', created at stack trace:\n" + creationStack,
            e);
      }
    }
  }
}
} // namespace org::apache::lucene::util