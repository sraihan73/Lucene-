using namespace std;

#include "DocValuesFieldUpdates.h"

namespace org::apache::lucene::index
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using InPlaceMergeSorter = org::apache::lucene::util::InPlaceMergeSorter;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using PagedMutable = org::apache::lucene::util::packed::PagedMutable;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

bool DocValuesFieldUpdates::Iterator::advanceExact(int target)
{
  throw make_shared<UnsupportedOperationException>();
}

int DocValuesFieldUpdates::Iterator::advance(int target)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t DocValuesFieldUpdates::Iterator::cost()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<BinaryDocValues> DocValuesFieldUpdates::Iterator::asBinaryDocValues(
    shared_ptr<Iterator> iterator)
{
  return make_shared<BinaryDocValuesAnonymousInnerClass>(iterator);
}

DocValuesFieldUpdates::Iterator::BinaryDocValuesAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass(
        shared_ptr<org::apache::lucene::index::DocValuesFieldUpdates::Iterator>
            iterator)
{
  this->iterator = iterator;
}

int DocValuesFieldUpdates::Iterator::BinaryDocValuesAnonymousInnerClass::docID()
{
  return iterator->docID();
}

shared_ptr<BytesRef> DocValuesFieldUpdates::Iterator::
    BinaryDocValuesAnonymousInnerClass::binaryValue()
{
  return iterator->binaryValue();
}

bool DocValuesFieldUpdates::Iterator::BinaryDocValuesAnonymousInnerClass::
    advanceExact(int target)
{
  return iterator->advanceExact(target);
}

int DocValuesFieldUpdates::Iterator::BinaryDocValuesAnonymousInnerClass::
    nextDoc()
{
  return iterator->nextDoc();
}

int DocValuesFieldUpdates::Iterator::BinaryDocValuesAnonymousInnerClass::
    advance(int target)
{
  return iterator->advance(target);
}

int64_t
DocValuesFieldUpdates::Iterator::BinaryDocValuesAnonymousInnerClass::cost()
{
  return iterator->cost();
}

shared_ptr<NumericDocValues>
DocValuesFieldUpdates::Iterator::asNumericDocValues(
    shared_ptr<Iterator> iterator)
{
  return make_shared<NumericDocValuesAnonymousInnerClass>(iterator);
}

DocValuesFieldUpdates::Iterator::NumericDocValuesAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass(
        shared_ptr<org::apache::lucene::index::DocValuesFieldUpdates::Iterator>
            iterator)
{
  this->iterator = iterator;
}

int64_t DocValuesFieldUpdates::Iterator::NumericDocValuesAnonymousInnerClass::
    longValue()
{
  return iterator->longValue();
}

bool DocValuesFieldUpdates::Iterator::NumericDocValuesAnonymousInnerClass::
    advanceExact(int target)
{
  throw make_shared<UnsupportedOperationException>();
}

int DocValuesFieldUpdates::Iterator::NumericDocValuesAnonymousInnerClass::
    docID()
{
  return iterator->docID();
}

int DocValuesFieldUpdates::Iterator::NumericDocValuesAnonymousInnerClass::
    nextDoc()
{
  return iterator->nextDoc();
}

int DocValuesFieldUpdates::Iterator::NumericDocValuesAnonymousInnerClass::
    advance(int target)
{
  return iterator->advance(target);
}

int64_t
DocValuesFieldUpdates::Iterator::NumericDocValuesAnonymousInnerClass::cost()
{
  return iterator->cost();
}

shared_ptr<Iterator> DocValuesFieldUpdates::mergedIterator(
    std::deque<std::shared_ptr<Iterator>> &subs)
{

  if (subs.size() == 1) {
    return subs[0];
  }

  shared_ptr<PriorityQueue<std::shared_ptr<Iterator>>> queue =
      make_shared<PriorityQueueAnonymousInnerClass>(subs.size());

  for (auto sub : subs) {
    if (sub->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
      queue->push_back(sub);
    }
  }

  if (queue->empty()) {
    return nullptr;
  }

  return make_shared<IteratorAnonymousInnerClass>(queue);
}

DocValuesFieldUpdates::PriorityQueueAnonymousInnerClass::
    PriorityQueueAnonymousInnerClass(shared_ptr<UnknownType> length)
    : org::apache::lucene::util::PriorityQueue<Iterator>(length)
{
}

bool DocValuesFieldUpdates::PriorityQueueAnonymousInnerClass::lessThan(
    shared_ptr<Iterator> a, shared_ptr<Iterator> b)
{
  // sort by smaller docID
  int cmp = Integer::compare(a->docID(), b->docID());
  if (cmp == 0) {
    // then by larger delGen
    cmp = Long::compare(b->delGen(), a->delGen());

    // delGens are unique across our subs:
    assert(cmp != 0);
  }

  return cmp < 0;
}

DocValuesFieldUpdates::IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
    shared_ptr<PriorityQueue<std::shared_ptr<Iterator>>> queue)
{
  this->queue = queue;
}

int DocValuesFieldUpdates::IteratorAnonymousInnerClass::nextDoc()
{
  // Advance all sub iterators past current doc
  while (true) {
    if (queue->empty()) {
      doc = DocIdSetIterator::NO_MORE_DOCS;
      break;
    }
    int newDoc = queue->top()->docID();
    if (newDoc != doc) {
      assert((newDoc > doc, L"doc=" + doc + L" newDoc=" + to_wstring(newDoc)));
      doc = newDoc;
      break;
    }
    if (queue->top()->nextDoc() == DocIdSetIterator::NO_MORE_DOCS) {
      queue->pop();
    } else {
      queue->updateTop();
    }
  }
  return doc;
}

int DocValuesFieldUpdates::IteratorAnonymousInnerClass::docID() { return doc; }

int64_t DocValuesFieldUpdates::IteratorAnonymousInnerClass::longValue()
{
  return queue->top()->longValue();
}

shared_ptr<BytesRef>
DocValuesFieldUpdates::IteratorAnonymousInnerClass::binaryValue()
{
  return queue->top()->binaryValue();
}

int64_t DocValuesFieldUpdates::IteratorAnonymousInnerClass::delGen()
{
  throw make_shared<UnsupportedOperationException>();
}

bool DocValuesFieldUpdates::IteratorAnonymousInnerClass::hasValue()
{
  return queue->top()->hasValue();
}

DocValuesFieldUpdates::DocValuesFieldUpdates(int maxDoc, int64_t delGen,
                                             const wstring &field,
                                             DocValuesType type)
    : field(field), type(type), delGen(delGen),
      bitsPerValue(PackedInts::bitsRequired(maxDoc - 1) + SHIFT), maxDoc(maxDoc)
{
  if (type == nullptr) {
    throw make_shared<NullPointerException>(L"DocValuesType must not be null");
  }
  docs = make_shared<PagedMutable>(1, PAGE_SIZE, bitsPerValue,
                                   PackedInts::COMPACT);
}

bool DocValuesFieldUpdates::getFinished() { return finished; }

// C++ WARNING: The following method was originally marked 'synchronized':
void DocValuesFieldUpdates::finish()
{
  if (finished) {
    throw make_shared<IllegalStateException>(L"already finished");
  }
  finished = true;

  // shrink wrap
  if (size_ < docs->size()) {
    resize(size_);
  }
  make_shared<InPlaceMergeSorterAnonymousInnerClass>(shared_from_this())
      .sort(0, size_);
}

DocValuesFieldUpdates::InPlaceMergeSorterAnonymousInnerClass::
    InPlaceMergeSorterAnonymousInnerClass(
        shared_ptr<DocValuesFieldUpdates> outerInstance)
{
  this->outerInstance = outerInstance;
}

void DocValuesFieldUpdates::InPlaceMergeSorterAnonymousInnerClass::swap(int i,
                                                                        int j)
{
  outerInstance->swap(i, j);
}

int DocValuesFieldUpdates::InPlaceMergeSorterAnonymousInnerClass::compare(int i,
                                                                          int j)
{
  // increasing docID order:
  // NOTE: we can have ties here, when the same docID was updated in the same
  // segment, in which case we rely on sort being stable and preserving original
  // order so the last update to that docID wins
  return Long::compare(outerInstance->docs->get(i),
                       outerInstance->docs->get(j));
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool DocValuesFieldUpdates::any() { return size_ > 0; }

// C++ WARNING: The following method was originally marked 'synchronized':
int DocValuesFieldUpdates::size() { return size_; }

// C++ WARNING: The following method was originally marked 'synchronized':
void DocValuesFieldUpdates::reset(int doc)
{
  addInternal(doc, HAS_NO_VALUE_MASK);
}

// C++ WARNING: The following method was originally marked 'synchronized':
int DocValuesFieldUpdates::add(int doc)
{
  return addInternal(doc, HAS_VALUE_MASK);
}

// C++ WARNING: The following method was originally marked 'synchronized':
int DocValuesFieldUpdates::addInternal(int doc, int64_t hasValueMask)
{
  if (finished) {
    throw make_shared<IllegalStateException>(L"already finished");
  }
  assert(doc < maxDoc);

  // TODO: if the Sorter interface changes to take long indexes, we can remove
  // that limitation
  if (size_ == numeric_limits<int>::max()) {
    throw make_shared<IllegalStateException>(
        L"cannot support more than Integer.MAX_VALUE doc/value entries");
  }
  // grow the structures to have room for more elements
  if (docs->size() == size_) {
    grow(size_ + 1);
  }
  docs->set(size_, ((static_cast<int64_t>(doc)) << SHIFT) | hasValueMask);
  ++size_;
  return size_ - 1;
}

void DocValuesFieldUpdates::swap(int i, int j)
{
  int64_t tmpDoc = docs->get(j);
  docs->set(j, docs->get(i));
  docs->set(i, tmpDoc);
}

void DocValuesFieldUpdates::grow(int size) { docs = docs->grow(size); }

void DocValuesFieldUpdates::resize(int size) { docs = docs->resize(size); }

void DocValuesFieldUpdates::ensureFinished()
{
  if (finished == false) {
    throw make_shared<IllegalStateException>(L"call finish first");
  }
}

int64_t DocValuesFieldUpdates::ramBytesUsed()
{
  return docs->ramBytesUsed() + RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
         2 * Integer::BYTES + 2 + Long::BYTES +
         RamUsageEstimator::NUM_BYTES_OBJECT_REF;
}

DocValuesFieldUpdates::AbstractIterator::AbstractIterator(
    int size, shared_ptr<PagedMutable> docs, int64_t delGen)
    : size(size), docs(docs), delGen(delGen)
{
}

int DocValuesFieldUpdates::AbstractIterator::nextDoc()
{
  if (idx >= size) {
    return doc = DocIdSetIterator::NO_MORE_DOCS;
  }
  int64_t longDoc = docs->get(idx);
  ++idx;
  while (idx < size && docs->get(idx) == longDoc) {
    // scan forward to last update to this doc
    ++idx;
  }
  hasValue_ = (longDoc & HAS_VALUE_MASK) > 0;
  if (hasValue_) {
    set(idx - 1);
  }
  doc = static_cast<int>(longDoc >> SHIFT);
  return doc;
}

int DocValuesFieldUpdates::AbstractIterator::docID() { return doc; }

int64_t DocValuesFieldUpdates::AbstractIterator::delGen() { return delGen_; }

bool DocValuesFieldUpdates::AbstractIterator::hasValue() { return hasValue_; }
} // namespace org::apache::lucene::index