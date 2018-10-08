using namespace std;

#include "FlushPolicy.h"

namespace org::apache::lucene::index
{
using ThreadState =
    org::apache::lucene::index::DocumentsWriterPerThreadPool::ThreadState;
using Directory = org::apache::lucene::store::Directory;
using InfoStream = org::apache::lucene::util::InfoStream;

void FlushPolicy::onUpdate(shared_ptr<DocumentsWriterFlushControl> control,
                           shared_ptr<ThreadState> state)
{
  onInsert(control, state);
  onDelete(control, state);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void FlushPolicy::init(shared_ptr<LiveIndexWriterConfig> indexWriterConfig)
{
  this->indexWriterConfig = indexWriterConfig;
  infoStream = indexWriterConfig->getInfoStream();
}

shared_ptr<ThreadState> FlushPolicy::findLargestNonPendingWriter(
    shared_ptr<DocumentsWriterFlushControl> control,
    shared_ptr<ThreadState> perThreadState)
{
  assert(perThreadState->dwpt->getNumDocsInRAM() > 0);
  // the dwpt which needs to be flushed eventually
  shared_ptr<ThreadState> maxRamUsingThreadState =
      control->findLargestNonPendingWriter();
  assert(assertMessage(
      L"set largest ram consuming thread pending on lower watermark"));
  return maxRamUsingThreadState;
}

bool FlushPolicy::assertMessage(const wstring &s)
{
  if (infoStream->isEnabled(L"FP")) {
    infoStream->message(L"FP", s);
  }
  return true;
}
} // namespace org::apache::lucene::index