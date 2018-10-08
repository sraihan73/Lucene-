using namespace std;

#include "FlushByRamOrCountsPolicy.h"

namespace org::apache::lucene::index
{
using ThreadState =
    org::apache::lucene::index::DocumentsWriterPerThreadPool::ThreadState;

void FlushByRamOrCountsPolicy::onDelete(
    shared_ptr<DocumentsWriterFlushControl> control,
    shared_ptr<ThreadState> state)
{
  if ((flushOnRAM() &&
       control->getDeleteBytesUsed() >
           1024 * 1024 * indexWriterConfig->getRAMBufferSizeMB())) {
    control->setApplyAllDeletes();
    if (infoStream->isEnabled(L"FP")) {
      infoStream->message(
          L"FP", L"force apply deletes bytesUsed=" +
                     to_wstring(control->getDeleteBytesUsed()) +
                     L" vs ramBufferMB=" +
                     to_wstring(indexWriterConfig->getRAMBufferSizeMB()));
    }
  }
}

void FlushByRamOrCountsPolicy::onInsert(
    shared_ptr<DocumentsWriterFlushControl> control,
    shared_ptr<ThreadState> state)
{
  if (flushOnDocCount() && state->dwpt->getNumDocsInRAM() >=
                               indexWriterConfig->getMaxBufferedDocs()) {
    // Flush this state by num docs
    control->setFlushPending(state);
  } else if (flushOnRAM()) { // flush by RAM
    constexpr int64_t limit = static_cast<int64_t>(
        indexWriterConfig->getRAMBufferSizeMB() * 1024.d * 1024.d);
    constexpr int64_t totalRam =
        control->activeBytes() + control->getDeleteBytesUsed();
    if (totalRam >= limit) {
      if (infoStream->isEnabled(L"FP")) {
        infoStream->message(
            L"FP", L"trigger flush: activeBytes=" +
                       to_wstring(control->activeBytes()) + L" deleteBytes=" +
                       to_wstring(control->getDeleteBytesUsed()) +
                       L" vs limit=" + to_wstring(limit));
      }
      markLargestWriterPending(control, state, totalRam);
    }
  }
}

void FlushByRamOrCountsPolicy::markLargestWriterPending(
    shared_ptr<DocumentsWriterFlushControl> control,
    shared_ptr<ThreadState> perThreadState,
    int64_t const currentBytesPerThread)
{
  shared_ptr<ThreadState> largestNonPendingWriter =
      findLargestNonPendingWriter(control, perThreadState);
  if (largestNonPendingWriter != nullptr) {
    control->setFlushPending(largestNonPendingWriter);
  }
}

bool FlushByRamOrCountsPolicy::flushOnDocCount()
{
  return indexWriterConfig->getMaxBufferedDocs() !=
         IndexWriterConfig::DISABLE_AUTO_FLUSH;
}

bool FlushByRamOrCountsPolicy::flushOnRAM()
{
  return indexWriterConfig->getRAMBufferSizeMB() !=
         IndexWriterConfig::DISABLE_AUTO_FLUSH;
}
} // namespace org::apache::lucene::index