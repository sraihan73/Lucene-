using namespace std;

#include "DocumentsWriterDeleteQueue.h"

namespace org::apache::lucene::index
{
using BinaryDocValuesUpdate =
    org::apache::lucene::index::DocValuesUpdate::BinaryDocValuesUpdate;
using NumericDocValuesUpdate =
    org::apache::lucene::index::DocValuesUpdate::NumericDocValuesUpdate;
using Query = org::apache::lucene::search::Query;
using Accountable = org::apache::lucene::util::Accountable;
using InfoStream = org::apache::lucene::util::InfoStream;

DocumentsWriterDeleteQueue::DocumentsWriterDeleteQueue(
    shared_ptr<InfoStream> infoStream)
    : DocumentsWriterDeleteQueue(infoStream, 0, 1)
{
  // seqNo must start at 1 because some APIs negate this to also return a
  // bool
}

DocumentsWriterDeleteQueue::DocumentsWriterDeleteQueue(
    shared_ptr<InfoStream> infoStream, int64_t generation,
    int64_t startSeqNo)
    : DocumentsWriterDeleteQueue(infoStream, new BufferedUpdates(L"global"),
                                 generation, startSeqNo)
{
}

DocumentsWriterDeleteQueue::DocumentsWriterDeleteQueue(
    shared_ptr<InfoStream> infoStream,
    shared_ptr<BufferedUpdates> globalBufferedUpdates, int64_t generation,
    int64_t startSeqNo)
    : globalSlice(make_shared<DeleteSlice>(tail)),
      globalBufferedUpdates(globalBufferedUpdates), generation(generation),
      nextSeqNo(make_shared<AtomicLong>(startSeqNo)), infoStream(infoStream)
{
  /*
   * we use a sentinel instance as our initial tail. No slice will ever try to
   * apply this tail since the head is always omitted.
   */
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: tail = new Node<>(null);
  tail = make_shared < Node < ? >> (nullptr); // sentinel
}

int64_t DocumentsWriterDeleteQueue::addDelete(deque<Query> &queries)
{
  int64_t seqNo = add(make_shared<QueryArrayNode>(queries));
  tryApplyGlobalSlice();
  return seqNo;
}

int64_t DocumentsWriterDeleteQueue::addDelete(deque<Term> &terms)
{
  int64_t seqNo = add(make_shared<TermArrayNode>(terms));
  tryApplyGlobalSlice();
  return seqNo;
}

int64_t DocumentsWriterDeleteQueue::addDocValuesUpdates(
    deque<DocValuesUpdate> &updates)
{
  int64_t seqNo = add(make_shared<DocValuesUpdatesNode>(updates));
  tryApplyGlobalSlice();
  return seqNo;
}

shared_ptr<Node<std::shared_ptr<Term>>>
DocumentsWriterDeleteQueue::newNode(shared_ptr<Term> term)
{
  return make_shared<TermNode>(term);
}

shared_ptr<Node<std::deque<std::shared_ptr<DocValuesUpdate>>>>
DocumentsWriterDeleteQueue::newNode(deque<DocValuesUpdate> &updates)
{
  return make_shared<DocValuesUpdatesNode>(updates);
}

template <typename T1>
int64_t DocumentsWriterDeleteQueue::add(shared_ptr<Node<T1>> deleteNode,
                                          shared_ptr<DeleteSlice> slice)
{
  int64_t seqNo = add(deleteNode);
  /*
   * this is an update request where the term is the updated documents
   * delTerm. in that case we need to guarantee that this insert is atomic
   * with regards to the given delete slice. This means if two threads try to
   * update the same document with in turn the same delTerm one of them must
   * win. By taking the node we have created for our del term as the new tail
   * it is guaranteed that if another thread adds the same right after us we
   * will apply this delete next time we update our slice and one of the two
   * competing updates wins!
   */
  slice->sliceTail = deleteNode;
  assert((slice->sliceHead != slice->sliceTail,
          L"slice head and tail must differ after add"));
  tryApplyGlobalSlice(); // TODO doing this each time is not necessary maybe
  // we can do it just every n times or so?

  return seqNo;
}

template <typename T1>
// C++ WARNING: The following method was originally marked 'synchronized':
int64_t DocumentsWriterDeleteQueue::add(shared_ptr<Node<T1>> newNode)
{
  tail->next = newNode;
  this->tail = newNode;
  return getNextSequenceNumber();
}

bool DocumentsWriterDeleteQueue::anyChanges()
{
  globalBufferLock->lock();
  try {
    /*
     * check if all items in the global slice were applied
     * and if the global slice is up-to-date
     * and if globalBufferedUpdates has changes
     */
    return globalBufferedUpdates->any() || !globalSlice->isEmpty() ||
           globalSlice->sliceTail != tail || tail->next != nullptr;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    globalBufferLock->unlock();
  }
}

void DocumentsWriterDeleteQueue::tryApplyGlobalSlice()
{
  if (globalBufferLock->tryLock()) {
    /*
     * The global buffer must be locked but we don't need to update them if
     * there is an update going on right now. It is sufficient to apply the
     * deletes that have been added after the current in-flight global slices
     * tail the next time we can get the lock!
     */
    try {
      if (updateSliceNoSeqNo(globalSlice)) {
        globalSlice->apply(globalBufferedUpdates, BufferedUpdates::MAX_INT);
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      globalBufferLock->unlock();
    }
  }
}

shared_ptr<FrozenBufferedUpdates>
DocumentsWriterDeleteQueue::freezeGlobalBuffer(
    shared_ptr<DeleteSlice> callerSlice) 
{
  globalBufferLock->lock();
  /*
   * Here we freeze the global buffer so we need to lock it, apply all
   * deletes in the queue and reset the global slice to let the GC prune the
   * queue.
   */
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: final Node<?> currentTail = tail;
  shared_ptr < Node < ? >> *const currentTail =
                            tail; // take the current tail make this local any
  // Changes after this call are applied later
  // and not relevant here
  if (callerSlice != nullptr) {
    // Update the callers slices so we are on the same page
    callerSlice->sliceTail = currentTail;
  }
  try {
    if (globalSlice->sliceTail != currentTail) {
      globalSlice->sliceTail = currentTail;
      globalSlice->apply(globalBufferedUpdates, BufferedUpdates::MAX_INT);
    }

    if (globalBufferedUpdates->any()) {
      shared_ptr<FrozenBufferedUpdates> *const packet =
          make_shared<FrozenBufferedUpdates>(infoStream, globalBufferedUpdates,
                                             nullptr);
      globalBufferedUpdates->clear();
      return packet;
    } else {
      return nullptr;
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    globalBufferLock->unlock();
  }
}

shared_ptr<DeleteSlice> DocumentsWriterDeleteQueue::newSlice()
{
  return make_shared<DeleteSlice>(tail);
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t DocumentsWriterDeleteQueue::updateSlice(shared_ptr<DeleteSlice> slice)
{
  int64_t seqNo = getNextSequenceNumber();
  if (slice->sliceTail != tail) {
    // new deletes arrived since we last checked
    slice->sliceTail = tail;
    seqNo = -seqNo;
  }
  return seqNo;
}

bool DocumentsWriterDeleteQueue::updateSliceNoSeqNo(
    shared_ptr<DeleteSlice> slice)
{
  if (slice->sliceTail != tail) {
    // new deletes arrived since we last checked
    slice->sliceTail = tail;
    return true;
  }
  return false;
}

template <typename T1>
DocumentsWriterDeleteQueue::DeleteSlice::DeleteSlice(
    shared_ptr<Node<T1>> currentTail)
{
  assert(currentTail != nullptr);
  /*
   * Initially this is a 0 length slice pointing to the 'current' tail of
   * the queue. Once we update the slice we only need to assign the tail and
   * have a new slice
   */
  sliceHead = sliceTail = currentTail;
}

void DocumentsWriterDeleteQueue::DeleteSlice::apply(
    shared_ptr<BufferedUpdates> del, int docIDUpto)
{
  if (sliceHead == sliceTail) {
    // 0 length slice
    return;
  }
  /*
   * When we apply a slice we take the head and get its next as our first
   * item to apply and continue until we applied the tail. If the head and
   * tail in this slice are not equal then there will be at least one more
   * non-null node in the slice!
   */
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: Node<?> current = sliceHead;
  shared_ptr < Node < ? >> current = sliceHead;
  do {
    current = current->next;
    assert((current != nullptr, L"slice property violated between the head on "
                                L"the tail must not be a null node"));
    current->apply(del, docIDUpto);
  } while (current != sliceTail);
  reset();
}

void DocumentsWriterDeleteQueue::DeleteSlice::reset()
{
  // Reset to a 0 length slice
  sliceHead = sliceTail;
}

template <typename T1>
bool DocumentsWriterDeleteQueue::DeleteSlice::isTail(shared_ptr<Node<T1>> node)
{
  return sliceTail == node;
}

bool DocumentsWriterDeleteQueue::DeleteSlice::isTailItem(any object)
{
  return sliceTail->item == object;
}

bool DocumentsWriterDeleteQueue::DeleteSlice::isEmpty()
{
  return sliceHead == sliceTail;
}

int DocumentsWriterDeleteQueue::numGlobalTermDeletes()
{
  return globalBufferedUpdates->numTermDeletes->get();
}

void DocumentsWriterDeleteQueue::clear()
{
  globalBufferLock->lock();
  try {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final Node<?> currentTail = tail;
    shared_ptr < Node < ? >> *const currentTail = tail;
    globalSlice->sliceHead = globalSlice->sliceTail = currentTail;
    globalBufferedUpdates->clear();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    globalBufferLock->unlock();
  }
}

DocumentsWriterDeleteQueue::TermNode::TermNode(shared_ptr<Term> term)
    : Node<Term>(term)
{
}

void DocumentsWriterDeleteQueue::TermNode::apply(
    shared_ptr<BufferedUpdates> bufferedDeletes, int docIDUpto)
{
  bufferedDeletes->addTerm(item, docIDUpto);
}

wstring DocumentsWriterDeleteQueue::TermNode::toString()
{
  return L"del=" + item;
}

DocumentsWriterDeleteQueue::QueryArrayNode::QueryArrayNode(
    std::deque<std::shared_ptr<Query>> &query)
    : Node<org::apache::lucene::search::Query[]>(query)
{
}

void DocumentsWriterDeleteQueue::QueryArrayNode::apply(
    shared_ptr<BufferedUpdates> bufferedUpdates, int docIDUpto)
{
  for (auto query : item) {
    bufferedUpdates->addQuery(query, docIDUpto);
  }
}

DocumentsWriterDeleteQueue::TermArrayNode::TermArrayNode(
    std::deque<std::shared_ptr<Term>> &term)
    : Node<Term[]>(term)
{
}

void DocumentsWriterDeleteQueue::TermArrayNode::apply(
    shared_ptr<BufferedUpdates> bufferedUpdates, int docIDUpto)
{
  for (auto term : item) {
    bufferedUpdates->addTerm(term, docIDUpto);
  }
}

wstring DocumentsWriterDeleteQueue::TermArrayNode::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"dels=" + Arrays->toString(item);
}

DocumentsWriterDeleteQueue::DocValuesUpdatesNode::DocValuesUpdatesNode(
    deque<DocValuesUpdate> &updates)
    : Node<DocValuesUpdate[]>(updates)
{
}

void DocumentsWriterDeleteQueue::DocValuesUpdatesNode::apply(
    shared_ptr<BufferedUpdates> bufferedUpdates, int docIDUpto)
{
  for (auto update : item) {
    switch (update->type) {
    case org::apache::lucene::index::DocValuesType::NUMERIC:
      bufferedUpdates->addNumericUpdate(
          std::static_pointer_cast<NumericDocValuesUpdate>(update), docIDUpto);
      break;
    case org::apache::lucene::index::DocValuesType::BINARY:
      bufferedUpdates->addBinaryUpdate(
          std::static_pointer_cast<BinaryDocValuesUpdate>(update), docIDUpto);
      break;
    default:
      throw invalid_argument(update->type +
                             L" DocValues updates not supported yet!");
    }
  }
}

bool DocumentsWriterDeleteQueue::DocValuesUpdatesNode::isDelete()
{
  return false;
}

wstring DocumentsWriterDeleteQueue::DocValuesUpdatesNode::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"docValuesUpdates: ");
  if (item.size() > 0) {
    sb->append(L"term=")->append(item[0]->term)->append(L"; updates: [");
    for (auto update : item) {
      sb->append(update->field)
          ->append(L':')
          ->append(update->valueToString())
          ->append(L',');
    }
    sb->setCharAt(sb->length() - 1, L']');
  }
  return sb->toString();
}

bool DocumentsWriterDeleteQueue::forceApplyGlobalSlice()
{
  globalBufferLock->lock();
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: final Node<?> currentTail = tail;
  shared_ptr < Node < ? >> *const currentTail = tail;
  try {
    if (globalSlice->sliceTail != currentTail) {
      globalSlice->sliceTail = currentTail;
      globalSlice->apply(globalBufferedUpdates, BufferedUpdates::MAX_INT);
    }
    return globalBufferedUpdates->any();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    globalBufferLock->unlock();
  }
}

int DocumentsWriterDeleteQueue::getBufferedUpdatesTermsSize()
{
  globalBufferLock->lock();
  try {
    forceApplyGlobalSlice();
    return globalBufferedUpdates->deleteTerms.size();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    globalBufferLock->unlock();
  }
}

int64_t DocumentsWriterDeleteQueue::ramBytesUsed()
{
  return globalBufferedUpdates->bytesUsed->get();
}

wstring DocumentsWriterDeleteQueue::toString()
{
  return L"DWDQ: [ generation: " + to_wstring(generation) + L" ]";
}

int64_t DocumentsWriterDeleteQueue::getNextSequenceNumber()
{
  int64_t seqNo = nextSeqNo->getAndIncrement();
  assert((seqNo < maxSeqNo, L"seqNo=" + to_wstring(seqNo) + L" vs maxSeqNo=" +
                                to_wstring(maxSeqNo)));
  return seqNo;
}

int64_t DocumentsWriterDeleteQueue::getLastSequenceNumber()
{
  return nextSeqNo->get() - 1;
}

void DocumentsWriterDeleteQueue::skipSequenceNumbers(int64_t jump)
{
  nextSeqNo->addAndGet(jump);
}
} // namespace org::apache::lucene::index