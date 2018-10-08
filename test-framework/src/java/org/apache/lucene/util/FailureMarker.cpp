using namespace std;

#include "FailureMarker.h"

namespace org::apache::lucene::util
{
using org::junit::runner::notification::Failure;
using org::junit::runner::notification::RunListener;
const shared_ptr<java::util::concurrent::atomic::AtomicInteger>
    FailureMarker::failures =
        make_shared<java::util::concurrent::atomic::AtomicInteger>();

void FailureMarker::testFailure(shared_ptr<Failure> failure) throw(
    runtime_error)
{
  failures->incrementAndGet();
}

bool FailureMarker::hadFailures() { return failures->get() > 0; }

int FailureMarker::getFailures() { return failures->get(); }

void FailureMarker::resetFailures() { failures->set(0); }
} // namespace org::apache::lucene::util