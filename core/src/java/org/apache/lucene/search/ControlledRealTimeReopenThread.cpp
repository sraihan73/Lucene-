using namespace std;

#include "ControlledRealTimeReopenThread.h"

namespace org::apache::lucene::search
{
using IndexWriter = org::apache::lucene::index::IndexWriter;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

ControlledRealTimeReopenThread<T>::HandleRefresh::HandleRefresh(
    shared_ptr<ControlledRealTimeReopenThread<T>> outerInstance)
    : outerInstance(outerInstance)
{
}

void ControlledRealTimeReopenThread<T>::HandleRefresh::beforeRefresh() {}

void ControlledRealTimeReopenThread<T>::HandleRefresh::afterRefresh(
    bool didRefresh)
{
  outerInstance->refreshDone();
}
} // namespace org::apache::lucene::search