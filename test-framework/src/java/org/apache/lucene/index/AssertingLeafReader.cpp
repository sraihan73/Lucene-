using namespace std;

#include "AssertingLeafReader.h"

namespace org::apache::lucene::index
{
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using StringHelper = org::apache::lucene::util::StringHelper;
using VirtualMethod = org::apache::lucene::util::VirtualMethod;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

void AssertingLeafReader::assertThread(const wstring &object,
                                       shared_ptr<Thread> creationThread)
{
  if (creationThread != Thread::currentThread()) {
    throw make_shared<AssertionError>(
        object + L" are only supposed to be consumed in " +
        L"the thread in which they have been acquired. But was acquired in " +
        creationThread + L" and consumed in " + Thread::currentThread() + L".");
  }
}

AssertingLeafReader::AssertingLeafReader(shared_ptr<LeafReader> in_)
    : FilterLeafReader(in_)
{
  // check some basic reader sanity
  assert(in_->maxDoc() >= 0);
  assert(in_->numDocs() <= in_->maxDoc());
  assert(in_->numDeletedDocs() + in_->numDocs() == in_->maxDoc());
  assert(!in_->hasDeletions() ||
         in_->numDeletedDocs() > 0 && in_->numDocs() < in_->maxDoc());

  shared_ptr<CacheHelper> coreCacheHelper = in_->getCoreCacheHelper();
  if (coreCacheHelper != nullptr) {
    coreCacheHelper->addClosedListener([&](any cacheKey) {
      constexpr any expectedKey = coreCacheHelper->getKey();
      assert((expectedKey == cacheKey,
              L"Core closed listener called on a different key " + expectedKey +
                  L" <> " + cacheKey));
    });
  }

  shared_ptr<CacheHelper> readerCacheHelper = in_->getReaderCacheHelper();
  if (readerCacheHelper != nullptr) {
    readerCacheHelper->addClosedListener([&](any cacheKey) {
      constexpr any expectedKey = readerCacheHelper->getKey();
      assert((expectedKey == cacheKey,
              L"Core closed listener called on a different key " + expectedKey +
                  L" <> " + cacheKey));
    });
  }
}

shared_ptr<Terms>
AssertingLeafReader::terms(const wstring &field) 
{
  shared_ptr<Terms> terms = FilterLeafReader::terms(field);
  return terms == nullptr ? nullptr : make_shared<AssertingTerms>(terms);
}

shared_ptr<Fields>
AssertingLeafReader::getTermVectors(int docID) 
{
  shared_ptr<Fields> fields = FilterLeafReader::getTermVectors(docID);
  return fields->empty() ? nullptr : make_shared<AssertingFields>(fields);
}

AssertingLeafReader::AssertingFields::AssertingFields(shared_ptr<Fields> in_)
    : FilterFields(in_)
{
}

shared_ptr<Iterator<wstring>> AssertingLeafReader::AssertingFields::iterator()
{
  org::apache::lucene::index::FilterLeafReader::FilterFields::const_iterator
      iterator = FilterFields::begin();
  assert(iterator != nullptr);
  return iterator;
}

shared_ptr<Terms> AssertingLeafReader::AssertingFields::terms(
    const wstring &field) 
{
  shared_ptr<Terms> terms = FilterFields::terms(field);
  return terms == nullptr ? nullptr : make_shared<AssertingTerms>(terms);
}

AssertingLeafReader::AssertingTerms::AssertingTerms(shared_ptr<Terms> in_)
    : FilterTerms(in_)
{
}

shared_ptr<TermsEnum> AssertingLeafReader::AssertingTerms::intersect(
    shared_ptr<CompiledAutomaton> automaton,
    shared_ptr<BytesRef> bytes) 
{
  shared_ptr<TermsEnum> termsEnum = in_->intersect(automaton, bytes);
  assert(termsEnum != nullptr);
  assert(bytes == nullptr || bytes->isValid());
  return make_shared<AssertingTermsEnum>(termsEnum);
}

shared_ptr<BytesRef>
AssertingLeafReader::AssertingTerms::getMin() 
{
  shared_ptr<BytesRef> v = in_->getMin();
  assert(v == nullptr || v->isValid());
  return v;
}

shared_ptr<BytesRef>
AssertingLeafReader::AssertingTerms::getMax() 
{
  shared_ptr<BytesRef> v = in_->getMax();
  assert(v == nullptr || v->isValid());
  return v;
}

shared_ptr<TermsEnum>
AssertingLeafReader::AssertingTerms::iterator() 
{
  shared_ptr<TermsEnum> termsEnum = FilterTerms::begin();
  assert(termsEnum != nullptr);
  return make_shared<AssertingTermsEnum>(termsEnum);
}

wstring AssertingLeafReader::AssertingTerms::toString()
{
  return L"AssertingTerms(" + in_ + L")";
}

const shared_ptr<
    org::apache::lucene::util::VirtualMethod<std::shared_ptr<TermsEnum>>>
    AssertingLeafReader::SEEK_EXACT = make_shared<
        org::apache::lucene::util::VirtualMethod<std::shared_ptr<TermsEnum>>>(
        TermsEnum::typeid, L"seekExact",
        org::apache::lucene::util::BytesRef::typeid);

AssertingLeafReader::AssertingTermsEnum::AssertingTermsEnum(
    shared_ptr<TermsEnum> in_)
    : FilterTermsEnum(in_),
      delegateOverridesSeekExact(SEEK_EXACT->isOverriddenAsOf(in_->getClass()))
{
}

shared_ptr<PostingsEnum> AssertingLeafReader::AssertingTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int flags) 
{
  assertThread(L"Terms enums", creationThread);
  assert((state == State::POSITIONED,
          L"docs(...) called on unpositioned TermsEnum"));

  // reuse if the codec reused
  shared_ptr<PostingsEnum> *const actualReuse;
  if (std::dynamic_pointer_cast<AssertingPostingsEnum>(reuse) != nullptr) {
    actualReuse = (std::static_pointer_cast<AssertingPostingsEnum>(reuse))->in_;
  } else {
    actualReuse.reset();
  }
  shared_ptr<PostingsEnum> docs = FilterTermsEnum::postings(actualReuse, flags);
  assert(docs != nullptr);
  if (docs == actualReuse) {
    // codec reused, reset asserting state
    (std::static_pointer_cast<AssertingPostingsEnum>(reuse))->reset();
    return reuse;
  } else {
    return make_shared<AssertingPostingsEnum>(docs);
  }
}

shared_ptr<BytesRef>
AssertingLeafReader::AssertingTermsEnum::next() 
{
  assertThread(L"Terms enums", creationThread);
  assert((state == State::INITIAL || state == State::POSITIONED,
          L"next() called on unpositioned TermsEnum"));
  shared_ptr<BytesRef> result = FilterTermsEnum::next();
  if (result == nullptr) {
    state = State::UNPOSITIONED;
  } else {
    assert(result->isValid());
    state = State::POSITIONED;
  }
  return result;
}

int64_t AssertingLeafReader::AssertingTermsEnum::ord() 
{
  assertThread(L"Terms enums", creationThread);
  assert(
      (state == State::POSITIONED, L"ord() called on unpositioned TermsEnum"));
  return FilterTermsEnum::ord();
}

int AssertingLeafReader::AssertingTermsEnum::docFreq() 
{
  assertThread(L"Terms enums", creationThread);
  assert((state == State::POSITIONED,
          L"docFreq() called on unpositioned TermsEnum"));
  return FilterTermsEnum::docFreq();
}

int64_t
AssertingLeafReader::AssertingTermsEnum::totalTermFreq() 
{
  assertThread(L"Terms enums", creationThread);
  assert((state == State::POSITIONED,
          L"totalTermFreq() called on unpositioned TermsEnum"));
  return FilterTermsEnum::totalTermFreq();
}

shared_ptr<BytesRef>
AssertingLeafReader::AssertingTermsEnum::term() 
{
  assertThread(L"Terms enums", creationThread);
  assert(
      (state == State::POSITIONED, L"term() called on unpositioned TermsEnum"));
  shared_ptr<BytesRef> ret = FilterTermsEnum::term();
  assert(ret == nullptr || ret->isValid());
  return ret;
}

void AssertingLeafReader::AssertingTermsEnum::seekExact(int64_t ord) throw(
    IOException)
{
  assertThread(L"Terms enums", creationThread);
  FilterTermsEnum::seekExact(ord);
  state = State::POSITIONED;
}

SeekStatus AssertingLeafReader::AssertingTermsEnum::seekCeil(
    shared_ptr<BytesRef> term) 
{
  assertThread(L"Terms enums", creationThread);
  assert(term->isValid());
  SeekStatus result = FilterTermsEnum::seekCeil(term);
  if (result == SeekStatus::END) {
    state = State::UNPOSITIONED;
  } else {
    state = State::POSITIONED;
  }
  return result;
}

bool AssertingLeafReader::AssertingTermsEnum::seekExact(
    shared_ptr<BytesRef> text) 
{
  assertThread(L"Terms enums", creationThread);
  assert(text->isValid());
  bool result;
  if (delegateOverridesSeekExact) {
    result = in_->seekExact(text);
  } else {
    result = FilterTermsEnum::seekExact(text);
  }
  if (result) {
    state = State::POSITIONED;
  } else {
    state = State::UNPOSITIONED;
  }
  return result;
}

shared_ptr<TermState>
AssertingLeafReader::AssertingTermsEnum::termState() 
{
  assertThread(L"Terms enums", creationThread);
  assert((state == State::POSITIONED,
          L"termState() called on unpositioned TermsEnum"));
  return in_->termState();
}

void AssertingLeafReader::AssertingTermsEnum::seekExact(
    shared_ptr<BytesRef> term, shared_ptr<TermState> state) 
{
  assertThread(L"Terms enums", creationThread);
  assert(term->isValid());
  in_->seekExact(term, state);
  this->state = State::POSITIONED;
}

wstring AssertingLeafReader::AssertingTermsEnum::toString()
{
  return L"AssertingTermsEnum(" + in_ + L")";
}

void AssertingLeafReader::AssertingTermsEnum::reset()
{
  state = State::INITIAL;
}

AssertingLeafReader::AssertingPostingsEnum::AssertingPostingsEnum(
    shared_ptr<PostingsEnum> in_)
    : FilterPostingsEnum(in_)
{
  this->doc = in_->docID();
}

int AssertingLeafReader::AssertingPostingsEnum::nextDoc() 
{
  assertThread(L"Docs enums", creationThread);
  assert((state != DocsEnumState::FINISHED,
          L"nextDoc() called after NO_MORE_DOCS"));
  int nextDoc = FilterPostingsEnum::nextDoc();
  assert((nextDoc > doc, L"backwards nextDoc from " + to_wstring(doc) +
                             L" to " + to_wstring(nextDoc) + L" " + in_));
  if (nextDoc == DocIdSetIterator::NO_MORE_DOCS) {
    state = DocsEnumState::FINISHED;
    positionMax = 0;
  } else {
    state = DocsEnumState::ITERATING;
    positionMax = FilterPostingsEnum::freq();
  }
  positionCount = 0;
  assert(FilterPostingsEnum::docID() == nextDoc);
  return doc = nextDoc;
}

int AssertingLeafReader::AssertingPostingsEnum::advance(int target) throw(
    IOException)
{
  assertThread(L"Docs enums", creationThread);
  assert((state != DocsEnumState::FINISHED,
          L"advance() called after NO_MORE_DOCS"));
  assert((target > doc, L"target must be > docID(), got " + to_wstring(target) +
                            L" <= " + to_wstring(doc)));
  int advanced = FilterPostingsEnum::advance(target);
  assert((advanced >= target, L"backwards advance from: " + to_wstring(target) +
                                  L" to: " + to_wstring(advanced)));
  if (advanced == DocIdSetIterator::NO_MORE_DOCS) {
    state = DocsEnumState::FINISHED;
    positionMax = 0;
  } else {
    state = DocsEnumState::ITERATING;
    positionMax = FilterPostingsEnum::freq();
  }
  positionCount = 0;
  assert(FilterPostingsEnum::docID() == advanced);
  return doc = advanced;
}

int AssertingLeafReader::AssertingPostingsEnum::docID()
{
  assertThread(L"Docs enums", creationThread);
  assert((doc == FilterPostingsEnum::docID(),
          L" invalid docID() in " + in_->getClass() + L" " +
              to_wstring(FilterPostingsEnum::docID()) + L" instead of " +
              to_wstring(doc)));
  return doc;
}

int AssertingLeafReader::AssertingPostingsEnum::freq() 
{
  assertThread(L"Docs enums", creationThread);
  assert((state != DocsEnumState::START,
          L"freq() called before nextDoc()/advance()"));
  assert(
      (state != DocsEnumState::FINISHED, L"freq() called after NO_MORE_DOCS"));
  int freq = FilterPostingsEnum::freq();
  assert(freq > 0);
  return freq;
}

int AssertingLeafReader::AssertingPostingsEnum::nextPosition() throw(
    IOException)
{
  assert((state != DocsEnumState::START,
          L"nextPosition() called before nextDoc()/advance()"));
  assert((state != DocsEnumState::FINISHED,
          L"nextPosition() called after NO_MORE_DOCS"));
  assert((positionCount < positionMax,
          L"nextPosition() called more than freq() times!"));
  int position = FilterPostingsEnum::nextPosition();
  assert((position >= 0 || position == -1,
          L"invalid position: " + to_wstring(position)));
  positionCount++;
  return position;
}

int AssertingLeafReader::AssertingPostingsEnum::startOffset() 
{
  assert((state != DocsEnumState::START,
          L"startOffset() called before nextDoc()/advance()"));
  assert((state != DocsEnumState::FINISHED,
          L"startOffset() called after NO_MORE_DOCS"));
  assert((positionCount > 0, L"startOffset() called before nextPosition()!"));
  return FilterPostingsEnum::startOffset();
}

int AssertingLeafReader::AssertingPostingsEnum::endOffset() 
{
  assert((state != DocsEnumState::START,
          L"endOffset() called before nextDoc()/advance()"));
  assert((state != DocsEnumState::FINISHED,
          L"endOffset() called after NO_MORE_DOCS"));
  assert((positionCount > 0, L"endOffset() called before nextPosition()!"));
  return FilterPostingsEnum::endOffset();
}

shared_ptr<BytesRef>
AssertingLeafReader::AssertingPostingsEnum::getPayload() 
{
  assert((state != DocsEnumState::START,
          L"getPayload() called before nextDoc()/advance()"));
  assert((state != DocsEnumState::FINISHED,
          L"getPayload() called after NO_MORE_DOCS"));
  assert((positionCount > 0, L"getPayload() called before nextPosition()!"));
  shared_ptr<BytesRef> payload = FilterPostingsEnum::getPayload();
  assert((payload == nullptr || payload->length > 0,
          L"getPayload() returned payload with invalid length!"));
  return payload;
}

void AssertingLeafReader::AssertingPostingsEnum::reset()
{
  state = DocsEnumState::START;
  doc = in_->docID();
  positionCount = positionMax = 0;
}

AssertingLeafReader::AssertingNumericDocValues::AssertingNumericDocValues(
    shared_ptr<NumericDocValues> in_, int maxDoc)
    : in_(in_), maxDoc(maxDoc)
{
  // should start unpositioned:
  assert(in_->docID() == -1);
}

int AssertingLeafReader::AssertingNumericDocValues::docID()
{
  assertThread(L"Numeric doc values", creationThread);
  return in_->docID();
}

int AssertingLeafReader::AssertingNumericDocValues::nextDoc() 
{
  assertThread(L"Numeric doc values", creationThread);
  int docID = in_->nextDoc();
  assert(docID > lastDocID);
  assert(docID == NO_MORE_DOCS || docID < maxDoc);
  assert(docID == in_->docID());
  lastDocID = docID;
  exists = docID != NO_MORE_DOCS;
  return docID;
}

int AssertingLeafReader::AssertingNumericDocValues::advance(int target) throw(
    IOException)
{
  assertThread(L"Numeric doc values", creationThread);
  assert(target >= 0);
  assert(target > in_->docID());
  int docID = in_->advance(target);
  assert(docID >= target);
  assert(docID == NO_MORE_DOCS || docID < maxDoc);
  lastDocID = docID;
  exists = docID != NO_MORE_DOCS;
  return docID;
}

bool AssertingLeafReader::AssertingNumericDocValues::advanceExact(
    int target) 
{
  assertThread(L"Numeric doc values", creationThread);
  assert(target >= 0);
  assert(target >= in_->docID());
  assert(target < maxDoc);
  exists = in_->advanceExact(target);
  assert(in_->docID() == target);
  lastDocID = target;
  return exists;
}

int64_t AssertingLeafReader::AssertingNumericDocValues::cost()
{
  assertThread(L"Numeric doc values", creationThread);
  int64_t cost = in_->cost();
  assert(cost >= 0);
  return cost;
}

int64_t
AssertingLeafReader::AssertingNumericDocValues::longValue() 
{
  assertThread(L"Numeric doc values", creationThread);
  assert(exists);
  return in_->longValue();
}

wstring AssertingLeafReader::AssertingNumericDocValues::toString()
{
  return L"AssertingNumericDocValues(" + in_ + L")";
}

AssertingLeafReader::AssertingBinaryDocValues::AssertingBinaryDocValues(
    shared_ptr<BinaryDocValues> in_, int maxDoc)
    : in_(in_), maxDoc(maxDoc)
{
  // should start unpositioned:
  assert(in_->docID() == -1);
}

int AssertingLeafReader::AssertingBinaryDocValues::docID()
{
  assertThread(L"Binary doc values", creationThread);
  return in_->docID();
}

int AssertingLeafReader::AssertingBinaryDocValues::nextDoc() 
{
  assertThread(L"Binary doc values", creationThread);
  int docID = in_->nextDoc();
  assert(docID > lastDocID);
  assert(docID == NO_MORE_DOCS || docID < maxDoc);
  assert(docID == in_->docID());
  lastDocID = docID;
  exists = docID != NO_MORE_DOCS;
  return docID;
}

int AssertingLeafReader::AssertingBinaryDocValues::advance(int target) throw(
    IOException)
{
  assertThread(L"Binary doc values", creationThread);
  assert(target >= 0);
  assert(target > in_->docID());
  int docID = in_->advance(target);
  assert(docID >= target);
  assert(docID == NO_MORE_DOCS || docID < maxDoc);
  lastDocID = docID;
  exists = docID != NO_MORE_DOCS;
  return docID;
}

bool AssertingLeafReader::AssertingBinaryDocValues::advanceExact(
    int target) 
{
  assertThread(L"Numeric doc values", creationThread);
  assert(target >= 0);
  assert(target >= in_->docID());
  assert(target < maxDoc);
  exists = in_->advanceExact(target);
  assert(in_->docID() == target);
  lastDocID = target;
  return exists;
}

int64_t AssertingLeafReader::AssertingBinaryDocValues::cost()
{
  assertThread(L"Binary doc values", creationThread);
  int64_t cost = in_->cost();
  assert(cost >= 0);
  return cost;
}

shared_ptr<BytesRef>
AssertingLeafReader::AssertingBinaryDocValues::binaryValue() 
{
  assertThread(L"Binary doc values", creationThread);
  assert(exists);
  return in_->binaryValue();
}

wstring AssertingLeafReader::AssertingBinaryDocValues::toString()
{
  return L"AssertingBinaryDocValues(" + in_ + L")";
}

AssertingLeafReader::AssertingSortedDocValues::AssertingSortedDocValues(
    shared_ptr<SortedDocValues> in_, int maxDoc)
    : in_(in_), maxDoc(maxDoc), valueCount(in_->getValueCount())
{
  assert(valueCount >= 0 && valueCount <= maxDoc);
}

int AssertingLeafReader::AssertingSortedDocValues::docID()
{
  assertThread(L"Sorted doc values", creationThread);
  return in_->docID();
}

int AssertingLeafReader::AssertingSortedDocValues::nextDoc() 
{
  assertThread(L"Sorted doc values", creationThread);
  int docID = in_->nextDoc();
  assert(docID > lastDocID);
  assert(docID == NO_MORE_DOCS || docID < maxDoc);
  assert(docID == in_->docID());
  lastDocID = docID;
  exists = docID != NO_MORE_DOCS;
  return docID;
}

int AssertingLeafReader::AssertingSortedDocValues::advance(int target) throw(
    IOException)
{
  assertThread(L"Sorted doc values", creationThread);
  assert(target >= 0);
  assert(target > in_->docID());
  int docID = in_->advance(target);
  assert(docID >= target);
  assert(docID == NO_MORE_DOCS || docID < maxDoc);
  lastDocID = docID;
  exists = docID != NO_MORE_DOCS;
  return docID;
}

bool AssertingLeafReader::AssertingSortedDocValues::advanceExact(
    int target) 
{
  assertThread(L"Numeric doc values", creationThread);
  assert(target >= 0);
  assert(target >= in_->docID());
  assert(target < maxDoc);
  exists = in_->advanceExact(target);
  assert(in_->docID() == target);
  lastDocID = target;
  return exists;
}

int64_t AssertingLeafReader::AssertingSortedDocValues::cost()
{
  assertThread(L"Sorted doc values", creationThread);
  int64_t cost = in_->cost();
  assert(cost >= 0);
  return cost;
}

int AssertingLeafReader::AssertingSortedDocValues::ordValue() 
{
  assertThread(L"Sorted doc values", creationThread);
  assert(exists);
  int ord = in_->ordValue();
  assert(ord >= -1 && ord < valueCount);
  return ord;
}

shared_ptr<BytesRef> AssertingLeafReader::AssertingSortedDocValues::lookupOrd(
    int ord) 
{
  assertThread(L"Sorted doc values", creationThread);
  assert(ord >= 0 && ord < valueCount);
  shared_ptr<BytesRef> *const result = in_->lookupOrd(ord);
  assert(result->isValid());
  return result;
}

int AssertingLeafReader::AssertingSortedDocValues::getValueCount()
{
  assertThread(L"Sorted doc values", creationThread);
  int valueCount = in_->getValueCount();
  assert(valueCount == this->valueCount); // should not change
  return valueCount;
}

shared_ptr<BytesRef>
AssertingLeafReader::AssertingSortedDocValues::binaryValue() 
{
  assertThread(L"Sorted doc values", creationThread);
  shared_ptr<BytesRef> *const result = in_->binaryValue();
  assert(result->isValid());
  return result;
}

int AssertingLeafReader::AssertingSortedDocValues::lookupTerm(
    shared_ptr<BytesRef> key) 
{
  assertThread(L"Sorted doc values", creationThread);
  assert(key->isValid());
  int result = in_->lookupTerm(key);
  assert(result < valueCount);
  assert(key->isValid());
  return result;
}

AssertingLeafReader::AssertingSortedNumericDocValues::
    AssertingSortedNumericDocValues(shared_ptr<SortedNumericDocValues> in_,
                                    int maxDoc)
    : in_(in_), maxDoc(maxDoc)
{
}

int AssertingLeafReader::AssertingSortedNumericDocValues::docID()
{
  return in_->docID();
}

int AssertingLeafReader::AssertingSortedNumericDocValues::nextDoc() throw(
    IOException)
{
  assertThread(L"Sorted numeric doc values", creationThread);
  int docID = in_->nextDoc();
  assert(docID > lastDocID);
  assert(docID == NO_MORE_DOCS || docID < maxDoc);
  assert(docID == in_->docID());
  lastDocID = docID;
  valueUpto = 0;
  exists = docID != NO_MORE_DOCS;
  return docID;
}

int AssertingLeafReader::AssertingSortedNumericDocValues::advance(
    int target) 
{
  assertThread(L"Sorted numeric doc values", creationThread);
  assert(target >= 0);
  assert(target > in_->docID());
  int docID = in_->advance(target);
  assert(docID == in_->docID());
  assert(docID >= target);
  assert(docID == NO_MORE_DOCS || docID < maxDoc);
  lastDocID = docID;
  valueUpto = 0;
  exists = docID != NO_MORE_DOCS;
  return docID;
}

bool AssertingLeafReader::AssertingSortedNumericDocValues::advanceExact(
    int target) 
{
  assertThread(L"Numeric doc values", creationThread);
  assert(target >= 0);
  assert(target >= in_->docID());
  assert(target < maxDoc);
  exists = in_->advanceExact(target);
  assert(in_->docID() == target);
  lastDocID = target;
  valueUpto = 0;
  return exists;
}

int64_t AssertingLeafReader::AssertingSortedNumericDocValues::cost()
{
  assertThread(L"Sorted numeric doc values", creationThread);
  int64_t cost = in_->cost();
  assert(cost >= 0);
  return cost;
}

int64_t
AssertingLeafReader::AssertingSortedNumericDocValues::nextValue() throw(
    IOException)
{
  assertThread(L"Sorted numeric doc values", creationThread);
  assert(exists);
  assert((valueUpto < in_->docValueCount(),
          L"valueUpto=" + to_wstring(valueUpto) + L" in.docValueCount()=" +
              to_wstring(in_->docValueCount())));
  valueUpto++;
  return in_->nextValue();
}

int AssertingLeafReader::AssertingSortedNumericDocValues::docValueCount()
{
  assertThread(L"Sorted numeric doc values", creationThread);
  assert(exists);
  assert(in_->docValueCount() > 0);
  return in_->docValueCount();
}

AssertingLeafReader::AssertingSortedSetDocValues::AssertingSortedSetDocValues(
    shared_ptr<SortedSetDocValues> in_, int maxDoc)
    : in_(in_), maxDoc(maxDoc), valueCount(in_->getValueCount())
{
  assert(valueCount >= 0);
}

int AssertingLeafReader::AssertingSortedSetDocValues::docID()
{
  assertThread(L"Sorted set doc values", creationThread);
  return in_->docID();
}

int AssertingLeafReader::AssertingSortedSetDocValues::nextDoc() throw(
    IOException)
{
  assertThread(L"Sorted set doc values", creationThread);
  int docID = in_->nextDoc();
  assert(docID > lastDocID);
  assert(docID == NO_MORE_DOCS || docID < maxDoc);
  assert(docID == in_->docID());
  lastDocID = docID;
  lastOrd = -2;
  exists = docID != NO_MORE_DOCS;
  return docID;
}

int AssertingLeafReader::AssertingSortedSetDocValues::advance(int target) throw(
    IOException)
{
  assertThread(L"Sorted set doc values", creationThread);
  assert(target >= 0);
  assert(target > in_->docID());
  int docID = in_->advance(target);
  assert(docID == in_->docID());
  assert(docID >= target);
  assert(docID == NO_MORE_DOCS || docID < maxDoc);
  lastDocID = docID;
  lastOrd = -2;
  exists = docID != NO_MORE_DOCS;
  return docID;
}

bool AssertingLeafReader::AssertingSortedSetDocValues::advanceExact(
    int target) 
{
  assertThread(L"Numeric doc values", creationThread);
  assert(target >= 0);
  assert(target >= in_->docID());
  assert(target < maxDoc);
  exists = in_->advanceExact(target);
  assert(in_->docID() == target);
  lastDocID = target;
  lastOrd = -2;
  return exists;
}

int64_t AssertingLeafReader::AssertingSortedSetDocValues::cost()
{
  assertThread(L"Sorted set doc values", creationThread);
  int64_t cost = in_->cost();
  assert(cost >= 0);
  return cost;
}

int64_t
AssertingLeafReader::AssertingSortedSetDocValues::nextOrd() 
{
  assertThread(L"Sorted set doc values", creationThread);
  assert(lastOrd != NO_MORE_ORDS);
  assert(exists);
  int64_t ord = in_->nextOrd();
  assert(ord < valueCount);
  assert(ord == NO_MORE_ORDS || ord > lastOrd);
  lastOrd = ord;
  return ord;
}

shared_ptr<BytesRef>
AssertingLeafReader::AssertingSortedSetDocValues::lookupOrd(
    int64_t ord) 
{
  assertThread(L"Sorted set doc values", creationThread);
  assert(ord >= 0 && ord < valueCount);
  shared_ptr<BytesRef> *const result = in_->lookupOrd(ord);
  assert(result->isValid());
  return result;
}

int64_t AssertingLeafReader::AssertingSortedSetDocValues::getValueCount()
{
  assertThread(L"Sorted set doc values", creationThread);
  int64_t valueCount = in_->getValueCount();
  assert(valueCount == this->valueCount); // should not change
  return valueCount;
}

int64_t AssertingLeafReader::AssertingSortedSetDocValues::lookupTerm(
    shared_ptr<BytesRef> key) 
{
  assertThread(L"Sorted set doc values", creationThread);
  assert(key->isValid());
  int64_t result = in_->lookupTerm(key);
  assert(result < valueCount);
  assert(key->isValid());
  return result;
}

AssertingLeafReader::AssertingPointValues::AssertingPointValues(
    shared_ptr<PointValues> in_, int maxDoc)
    : in_(in_)
{
  assertStats(maxDoc);
}

shared_ptr<PointValues> AssertingLeafReader::AssertingPointValues::getWrapped()
{
  return in_;
}

void AssertingLeafReader::AssertingPointValues::assertStats(int maxDoc)
{
  assert(in_->size() > 0);
  assert(in_->getDocCount() > 0);
  assert(in_->getDocCount() <= in_->size());
  assert(in_->getDocCount() <= maxDoc);
}

void AssertingLeafReader::AssertingPointValues::intersect(
    shared_ptr<IntersectVisitor> visitor) 
{
  in_->intersect(make_shared<AssertingIntersectVisitor>(
      in_->getNumDimensions(), in_->getBytesPerDimension(), visitor));
}

int64_t AssertingLeafReader::AssertingPointValues::estimatePointCount(
    shared_ptr<IntersectVisitor> visitor)
{
  int64_t cost = in_->estimatePointCount(visitor);
  assert(cost >= 0);
  return cost;
}

std::deque<char>
AssertingLeafReader::AssertingPointValues::getMinPackedValue() throw(
    IOException)
{
  return Objects::requireNonNull(in_->getMinPackedValue());
}

std::deque<char>
AssertingLeafReader::AssertingPointValues::getMaxPackedValue() throw(
    IOException)
{
  return Objects::requireNonNull(in_->getMaxPackedValue());
}

int AssertingLeafReader::AssertingPointValues::getNumDimensions() throw(
    IOException)
{
  return in_->getNumDimensions();
}

int AssertingLeafReader::AssertingPointValues::getBytesPerDimension() throw(
    IOException)
{
  return in_->getBytesPerDimension();
}

int64_t AssertingLeafReader::AssertingPointValues::size()
{
  return in_->size();
}

int AssertingLeafReader::AssertingPointValues::getDocCount()
{
  return in_->getDocCount();
}

AssertingLeafReader::AssertingIntersectVisitor::AssertingIntersectVisitor(
    int numDims, int bytesPerDim, shared_ptr<IntersectVisitor> in_)
    : in_(in_), numDims(numDims), bytesPerDim(bytesPerDim),
      lastMinPackedValue(std::deque<char>(numDims * bytesPerDim)),
      lastMaxPackedValue(std::deque<char>(numDims * bytesPerDim))
{
  if (numDims == 1) {
    lastDocValue = std::deque<char>(bytesPerDim);
  } else {
    lastDocValue.clear();
  }
}

void AssertingLeafReader::AssertingIntersectVisitor::visit(int docID) throw(
    IOException)
{
  assert((--docBudget >= 0,
          L"called add() more times than the last call to grow() reserved"));

  // This method, not filtering each hit, should only be invoked when the cell
  // is inside the query shape:
  assert(lastCompareResult == Relation::CELL_INSIDE_QUERY);
  in_->visit(docID);
}

void AssertingLeafReader::AssertingIntersectVisitor::visit(
    int docID, std::deque<char> &packedValue) 
{
  assert((--docBudget >= 0,
          L"called add() more times than the last call to grow() reserved"));

  // This method, to filter each doc's value, should only be invoked when the
  // cell crosses the query shape:
  assert(lastCompareResult == PointValues::Relation::CELL_CROSSES_QUERY);

  // This doc's packed value should be contained in the last cell passed to
  // compare:
  for (int dim = 0; dim < numDims; dim++) {
    assert((StringHelper::compare(bytesPerDim, lastMinPackedValue,
                                  dim * bytesPerDim, packedValue,
                                  dim * bytesPerDim) <= 0,
            L"dim=" + to_wstring(dim) + L" of " + to_wstring(numDims) +
                L" value=" + make_shared<BytesRef>(packedValue)));
    assert((StringHelper::compare(bytesPerDim, lastMaxPackedValue,
                                  dim * bytesPerDim, packedValue,
                                  dim * bytesPerDim) >= 0,
            L"dim=" + to_wstring(dim) + L" of " + to_wstring(numDims) +
                L" value=" + make_shared<BytesRef>(packedValue)));
  }

  // TODO: we should assert that this "matches" whatever relation the last call
  // to compare had returned
  assert(packedValue.size() == numDims * bytesPerDim);
  if (numDims == 1) {
    int cmp =
        StringHelper::compare(bytesPerDim, lastDocValue, 0, packedValue, 0);
    if (cmp < 0) {
      // ok
    } else if (cmp == 0) {
      assert((lastDocID <= docID,
              L"doc ids are out of order when point values are the same!"));
    } else {
      // out of order!
      assert((false, L"point values are out of order"));
    }
    System::arraycopy(packedValue, 0, lastDocValue, 0, bytesPerDim);
    lastDocID = docID;
  }
  in_->visit(docID, packedValue);
}

void AssertingLeafReader::AssertingIntersectVisitor::grow(int count)
{
  in_->grow(count);
  docBudget = count;
}

Relation AssertingLeafReader::AssertingIntersectVisitor::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  for (int dim = 0; dim < numDims; dim++) {
    assert(
        (StringHelper::compare(bytesPerDim, minPackedValue, dim * bytesPerDim,
                               maxPackedValue, dim * bytesPerDim) <= 0));
  }
  System::arraycopy(maxPackedValue, 0, lastMaxPackedValue, 0,
                    numDims * bytesPerDim);
  System::arraycopy(minPackedValue, 0, lastMinPackedValue, 0,
                    numDims * bytesPerDim);
  lastCompareResult = in_->compare(minPackedValue, maxPackedValue);
  return lastCompareResult;
}

shared_ptr<NumericDocValues> AssertingLeafReader::getNumericDocValues(
    const wstring &field) 
{
  shared_ptr<NumericDocValues> dv =
      FilterLeafReader::getNumericDocValues(field);
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (dv != nullptr) {
    assert(fi != nullptr);
    assert(fi->getDocValuesType() == DocValuesType::NUMERIC);
    return make_shared<AssertingNumericDocValues>(dv, maxDoc());
  } else {
    assert(fi == nullptr || fi->getDocValuesType() != DocValuesType::NUMERIC);
    return nullptr;
  }
}

shared_ptr<BinaryDocValues>
AssertingLeafReader::getBinaryDocValues(const wstring &field) 
{
  shared_ptr<BinaryDocValues> dv = FilterLeafReader::getBinaryDocValues(field);
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (dv != nullptr) {
    assert(fi != nullptr);
    assert(fi->getDocValuesType() == DocValuesType::BINARY);
    return make_shared<AssertingBinaryDocValues>(dv, maxDoc());
  } else {
    assert(fi == nullptr || fi->getDocValuesType() != DocValuesType::BINARY);
    return nullptr;
  }
}

shared_ptr<SortedDocValues>
AssertingLeafReader::getSortedDocValues(const wstring &field) 
{
  shared_ptr<SortedDocValues> dv = FilterLeafReader::getSortedDocValues(field);
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (dv != nullptr) {
    assert(fi != nullptr);
    assert(fi->getDocValuesType() == DocValuesType::SORTED);
    return make_shared<AssertingSortedDocValues>(dv, maxDoc());
  } else {
    assert(fi == nullptr || fi->getDocValuesType() != DocValuesType::SORTED);
    return nullptr;
  }
}

shared_ptr<SortedNumericDocValues>
AssertingLeafReader::getSortedNumericDocValues(const wstring &field) throw(
    IOException)
{
  shared_ptr<SortedNumericDocValues> dv =
      FilterLeafReader::getSortedNumericDocValues(field);
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (dv != nullptr) {
    assert(fi != nullptr);
    assert(fi->getDocValuesType() == DocValuesType::SORTED_NUMERIC);
    return make_shared<AssertingSortedNumericDocValues>(dv, maxDoc());
  } else {
    assert(fi == nullptr ||
           fi->getDocValuesType() != DocValuesType::SORTED_NUMERIC);
    return nullptr;
  }
}

shared_ptr<SortedSetDocValues> AssertingLeafReader::getSortedSetDocValues(
    const wstring &field) 
{
  shared_ptr<SortedSetDocValues> dv =
      FilterLeafReader::getSortedSetDocValues(field);
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (dv != nullptr) {
    assert(fi != nullptr);
    assert(fi->getDocValuesType() == DocValuesType::SORTED_SET);
    return make_shared<AssertingSortedSetDocValues>(dv, maxDoc());
  } else {
    assert(fi == nullptr ||
           fi->getDocValuesType() != DocValuesType::SORTED_SET);
    return nullptr;
  }
}

shared_ptr<NumericDocValues>
AssertingLeafReader::getNormValues(const wstring &field) 
{
  shared_ptr<NumericDocValues> dv = FilterLeafReader::getNormValues(field);
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (dv != nullptr) {
    assert(fi != nullptr);
    assert(fi->hasNorms());
    return make_shared<AssertingNumericDocValues>(dv, maxDoc());
  } else {
    assert(fi == nullptr || fi->hasNorms() == false);
    return nullptr;
  }
}

shared_ptr<PointValues>
AssertingLeafReader::getPointValues(const wstring &field) 
{
  shared_ptr<PointValues> values = in_->getPointValues(field);
  if (values == nullptr) {
    return nullptr;
  }
  return make_shared<AssertingPointValues>(values, maxDoc());
}

AssertingLeafReader::AssertingBits::AssertingBits(shared_ptr<Bits> in_)
    : in_(in_)
{
}

bool AssertingLeafReader::AssertingBits::get(int index)
{
  assertThread(L"Bits", creationThread);
  assert(index >= 0 && index < length());
  return in_->get(index);
}

int AssertingLeafReader::AssertingBits::length()
{
  assertThread(L"Bits", creationThread);
  return in_->length();
}

shared_ptr<Bits> AssertingLeafReader::getLiveDocs()
{
  shared_ptr<Bits> liveDocs = FilterLeafReader::getLiveDocs();
  if (liveDocs != nullptr) {
    assert(maxDoc() == liveDocs->length());
    liveDocs = make_shared<AssertingBits>(liveDocs);
  } else {
    assert(maxDoc() == numDocs());
    assert(!hasDeletions());
  }
  return liveDocs;
}

shared_ptr<CacheHelper> AssertingLeafReader::getCoreCacheHelper()
{
  return in_->getCoreCacheHelper();
}

shared_ptr<CacheHelper> AssertingLeafReader::getReaderCacheHelper()
{
  return in_->getReaderCacheHelper();
}
} // namespace org::apache::lucene::index