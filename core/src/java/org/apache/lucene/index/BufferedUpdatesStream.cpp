using namespace std;

#include "BufferedUpdatesStream.h"

namespace org::apache::lucene::index
{
using IOContext = org::apache::lucene::store::IOContext;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;

BufferedUpdatesStream::BufferedUpdatesStream(shared_ptr<InfoStream> infoStream)
    : finishedSegments(make_shared<FinishedSegments>(infoStream)),
      infoStream(infoStream)
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t BufferedUpdatesStream::push(shared_ptr<FrozenBufferedUpdates> packet)
{
  /*
   * The insert operation must be atomic. If we let threads increment the gen
   * and push the packet afterwards we risk that packets are out of order.
   * With DWPT this is possible if two or more flushes are racing for pushing
   * updates. If the pushed packets get our of order would loose documents
   * since deletes are applied to the wrong segments.
   */
  packet->setDelGen(nextGen++);
  assert(packet->any());
  assert(checkDeleteStats());

  updates->add(packet);
  numTerms_->addAndGet(packet->numTermDeletes);
  bytesUsed->addAndGet(packet->bytesUsed);
  if (infoStream->isEnabled(L"BD")) {
    infoStream->message(
        L"BD",
        wstring::format(
            Locale::ROOT,
            L"push new packet (%s), packetCount=%d, bytesUsed=%.3f MB", packet,
            updates->size(), bytesUsed->get() / 1024.0 / 1024.0));
  }
  assert(checkDeleteStats());

  return packet->delGen();
}

// C++ WARNING: The following method was originally marked 'synchronized':
int BufferedUpdatesStream::getPendingUpdatesCount() { return updates->size(); }

// C++ WARNING: The following method was originally marked 'synchronized':
void BufferedUpdatesStream::clear()
{
  updates->clear();
  nextGen = 1;
  finishedSegments->clear();
  numTerms_->set(0);
  bytesUsed->set(0);
}

bool BufferedUpdatesStream::any() { return bytesUsed->get() != 0; }

int BufferedUpdatesStream::numTerms() { return numTerms_->get(); }

int64_t BufferedUpdatesStream::ramBytesUsed() { return bytesUsed->get(); }

BufferedUpdatesStream::ApplyDeletesResult::ApplyDeletesResult(
    bool anyDeletes, deque<std::shared_ptr<SegmentCommitInfo>> &allDeleted)
    : anyDeletes(anyDeletes), allDeleted(allDeleted)
{
}

void BufferedUpdatesStream::waitApplyAll(shared_ptr<IndexWriter> writer) throw(
    IOException)
{
  assert(Thread::holdsLock(writer) == false);
  shared_ptr<Set<std::shared_ptr<FrozenBufferedUpdates>>> waitFor;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this()) { waitFor = unordered_set<>(updates); }

  waitApply(waitFor, writer);
}

bool BufferedUpdatesStream::stillRunning(int64_t delGen)
{
  return finishedSegments->stillRunning(delGen);
}

void BufferedUpdatesStream::finishedSegment(int64_t delGen)
{
  finishedSegments->finishedSegment(delGen);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void BufferedUpdatesStream::finished(shared_ptr<FrozenBufferedUpdates> packet)
{
  // TODO: would be a bit more memory efficient to track this per-segment, so
  // when each segment writes it writes all packets finished for it, rather than
  // only recording here, across all segments.  But, more complex code, and more
  // CPU, and maybe not so much impact in practice?
  assert((packet->applied->getCount() == 1, L"packet=" + packet));

  packet->applied->countDown();

  updates->remove(packet);
  numTerms_->addAndGet(-packet->numTermDeletes);
  assert(
      (numTerms_->get() >= 0, L"numTerms=" + numTerms_ + L" packet=" + packet));

  bytesUsed->addAndGet(-packet->bytesUsed);

  finishedSegment(packet->delGen());
}

int64_t BufferedUpdatesStream::getCompletedDelGen()
{
  return finishedSegments->getCompletedDelGen();
}

void BufferedUpdatesStream::waitApplyForMerge(
    deque<std::shared_ptr<SegmentCommitInfo>> &mergeInfos,
    shared_ptr<IndexWriter> writer) 
{
  int64_t maxDelGen = numeric_limits<int64_t>::min();
  for (auto info : mergeInfos) {
    maxDelGen = max(maxDelGen, info->getBufferedDeletesGen());
  }

  shared_ptr<Set<std::shared_ptr<FrozenBufferedUpdates>>> waitFor =
      unordered_set<std::shared_ptr<FrozenBufferedUpdates>>();
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    for (auto packet : updates) {
      if (packet->delGen() <= maxDelGen) {
        // We must wait for this packet before finishing the merge because its
        // deletes apply to a subset of the segments being merged:
        waitFor->add(packet);
      }
    }
  }

  if (infoStream->isEnabled(L"BD")) {
    infoStream->message(L"BD", L"waitApplyForMerge: " + waitFor->size() +
                                   L" packets, " + mergeInfos.size() +
                                   L" merging segments");
  }

  waitApply(waitFor, writer);
}

void BufferedUpdatesStream::waitApply(
    shared_ptr<Set<std::shared_ptr<FrozenBufferedUpdates>>> waitFor,
    shared_ptr<IndexWriter> writer) 
{

  int64_t startNS = System::nanoTime();

  int packetCount = waitFor->size();

  if (waitFor->isEmpty()) {
    if (infoStream->isEnabled(L"BD")) {
      infoStream->message(L"BD", L"waitApply: no deletes to apply");
    }
    return;
  }

  if (infoStream->isEnabled(L"BD")) {
    infoStream->message(L"BD", L"waitApply: " + waitFor->size() +
                                   L" packets: " + waitFor);
  }

  int64_t totalDelCount = 0;
  for (auto packet : waitFor) {
    // Frozen packets are now resolved, concurrently, by the indexing threads
    // that create them, by adding a DocumentsWriter.ResolveUpdatesEvent to the
    // events queue, but if we get here and the packet is not yet resolved, we
    // resolve it now ourselves:
    packet->apply(writer);
    totalDelCount += packet->totalDelCount;
  }

  if (infoStream->isEnabled(L"BD")) {
    infoStream->message(
        L"BD", wstring::format(Locale::ROOT,
                               L"waitApply: done %d packets; totalDelCount=%d; "
                               L"totBytesUsed=%d; took %.2f msec",
                               packetCount, totalDelCount, bytesUsed->get(),
                               (System::nanoTime() - startNS) / 1000000.0));
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t BufferedUpdatesStream::getNextGen() { return nextGen++; }

BufferedUpdatesStream::SegmentState::SegmentState(
    shared_ptr<ReadersAndUpdates> rld,
    IOUtils::IOConsumer<std::shared_ptr<ReadersAndUpdates>> onClose,
    shared_ptr<SegmentCommitInfo> info) 
    : delGen(info->getBufferedDeletesGen()), rld(rld),
      reader(rld->getReader(IOContext::READ)),
      startDelCount(rld->getDelCount()), onClose(onClose)
{
}

wstring BufferedUpdatesStream::SegmentState::toString()
{
  return L"SegmentState(" + rld->info + L")";
}

BufferedUpdatesStream::SegmentState::~SegmentState()
{
  IOUtils::close({[&]() { rld->release(reader); }, [&]() { onClose(rld); }});
}

bool BufferedUpdatesStream::checkDeleteStats()
{
  int numTerms2 = 0;
  int64_t bytesUsed2 = 0;
  for (auto packet : updates) {
    numTerms2 += packet->numTermDeletes;
    bytesUsed2 += packet->bytesUsed;
  }
  assert((numTerms2 == numTerms_->get(),
          L"numTerms2=" + to_wstring(numTerms2) + L" vs " + numTerms_->get()));
  assert((bytesUsed2 == bytesUsed->get(),
          L"bytesUsed2=" + to_wstring(bytesUsed2) + L" vs " + bytesUsed));
  return true;
}

BufferedUpdatesStream::FinishedSegments::FinishedSegments(
    shared_ptr<InfoStream> infoStream)
    : infoStream(infoStream)
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
void BufferedUpdatesStream::FinishedSegments::clear()
{
  finishedDelGens->clear();
  completedDelGen = 0;
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool BufferedUpdatesStream::FinishedSegments::stillRunning(int64_t delGen)
{
  return delGen > completedDelGen && finishedDelGens->contains(delGen) == false;
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t BufferedUpdatesStream::FinishedSegments::getCompletedDelGen()
{
  return completedDelGen;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void BufferedUpdatesStream::FinishedSegments::finishedSegment(int64_t delGen)
{
  finishedDelGens->add(delGen);
  while (true) {
    if (finishedDelGens->contains(completedDelGen + 1)) {
      finishedDelGens->remove(completedDelGen + 1);
      completedDelGen++;
    } else {
      break;
    }
  }

  if (infoStream->isEnabled(L"BD")) {
    infoStream->message(L"BD", L"finished packet delGen=" + to_wstring(delGen) +
                                   L" now completedDelGen=" +
                                   to_wstring(completedDelGen));
  }
}
} // namespace org::apache::lucene::index