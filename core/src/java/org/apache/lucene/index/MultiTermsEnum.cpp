using namespace std;

#include "MultiTermsEnum.h"

namespace org::apache::lucene::index
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
const shared_ptr<java::util::Comparator<std::shared_ptr<TermsEnumWithSlice>>>
    MultiTermsEnum::INDEX_COMPARATOR =
        make_shared<ComparatorAnonymousInnerClass>();

MultiTermsEnum::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass()
{
}

int MultiTermsEnum::ComparatorAnonymousInnerClass::compare(
    shared_ptr<TermsEnumWithSlice> o1, shared_ptr<TermsEnumWithSlice> o2)
{
  return o1->index - o2->index;
}

std::deque<std::shared_ptr<TermsEnumIndex>> const
    MultiTermsEnum::TermsEnumIndex::EMPTY_ARRAY =
        std::deque<std::shared_ptr<TermsEnumIndex>>(0);

MultiTermsEnum::TermsEnumIndex::TermsEnumIndex(shared_ptr<TermsEnum> termsEnum,
                                               int subIndex)
    : subIndex(subIndex), termsEnum(termsEnum)
{
}

int MultiTermsEnum::getMatchCount() { return numTop; }

std::deque<std::shared_ptr<TermsEnumWithSlice>> MultiTermsEnum::getMatchArray()
{
  return top;
}

MultiTermsEnum::MultiTermsEnum(
    std::deque<std::shared_ptr<ReaderSlice>> &slices)
    : queue(make_shared<TermMergeQueue>(slices.size())),
      subs(std::deque<std::shared_ptr<TermsEnumWithSlice>>(slices.size())),
      currentSubs(
          std::deque<std::shared_ptr<TermsEnumWithSlice>>(slices.size())),
      top(std::deque<std::shared_ptr<TermsEnumWithSlice>>(slices.size())),
      subDocs(std::deque<std::shared_ptr<MultiPostingsEnum::EnumWithSlice>>(
          slices.size()))
{
  for (int i = 0; i < slices.size(); i++) {
    subs[i] = make_shared<TermsEnumWithSlice>(i, slices[i]);
    subDocs[i] = make_shared<MultiPostingsEnum::EnumWithSlice>();
    subDocs[i]->slice = slices[i];
  }
}

shared_ptr<BytesRef> MultiTermsEnum::term() { return current; }

shared_ptr<TermsEnum>
MultiTermsEnum::reset(std::deque<std::shared_ptr<TermsEnumIndex>>
                          &termsEnumsIndex) 
{
  assert(termsEnumsIndex.size() <= top.size());
  numSubs = 0;
  numTop = 0;
  queue->clear();
  for (int i = 0; i < termsEnumsIndex.size(); i++) {

    shared_ptr<TermsEnumIndex> *const termsEnumIndex = termsEnumsIndex[i];
    assert(termsEnumIndex != nullptr);

    shared_ptr<BytesRef> *const term = termsEnumIndex->termsEnum->next();
    if (term != nullptr) {
      shared_ptr<TermsEnumWithSlice> *const entry =
          subs[termsEnumIndex->subIndex];
      entry->reset(termsEnumIndex->termsEnum, term);
      queue->push_back(entry);
      currentSubs[numSubs++] = entry;
    } else {
      // field has no terms
    }
  }

  if (queue->empty()) {
    return TermsEnum::EMPTY;
  } else {
    return shared_from_this();
  }
}

bool MultiTermsEnum::seekExact(shared_ptr<BytesRef> term) 
{
  queue->clear();
  numTop = 0;

  bool seekOpt = false;
  if (lastSeek != nullptr && lastSeek->compareTo(term) <= 0) {
    seekOpt = true;
  }

  lastSeek.reset();
  lastSeekExact = true;

  for (int i = 0; i < numSubs; i++) {
    constexpr bool status;
    // LUCENE-2130: if we had just seek'd already, prior
    // to this seek, and the new seek term is after the
    // previous one, don't try to re-seek this sub if its
    // current term is already beyond this new seek term.
    // Doing so is a waste because this sub will simply
    // seek to the same spot.
    if (seekOpt) {
      shared_ptr<BytesRef> *const curTerm = currentSubs[i]->current;
      if (curTerm != nullptr) {
        constexpr int cmp = term->compareTo(curTerm);
        if (cmp == 0) {
          status = true;
        } else if (cmp < 0) {
          status = false;
        } else {
          status = currentSubs[i]->terms.seekExact(term);
        }
      } else {
        status = false;
      }
    } else {
      status = currentSubs[i]->terms.seekExact(term);
    }

    if (status) {
      top[numTop++] = currentSubs[i];
      current = currentSubs[i]->current = currentSubs[i]->terms.term();
      assert(term->equals(currentSubs[i]->current));
    }
  }

  // if at least one sub had exact match to the requested
  // term then we found match
  return numTop > 0;
}

SeekStatus
MultiTermsEnum::seekCeil(shared_ptr<BytesRef> term) 
{
  queue->clear();
  numTop = 0;
  lastSeekExact = false;

  bool seekOpt = false;
  if (lastSeek != nullptr && lastSeek->compareTo(term) <= 0) {
    seekOpt = true;
  }

  lastSeekScratch->copyBytes(term);
  lastSeek = lastSeekScratch->get();

  for (int i = 0; i < numSubs; i++) {
    constexpr SeekStatus status;
    // LUCENE-2130: if we had just seek'd already, prior
    // to this seek, and the new seek term is after the
    // previous one, don't try to re-seek this sub if its
    // current term is already beyond this new seek term.
    // Doing so is a waste because this sub will simply
    // seek to the same spot.
    if (seekOpt) {
      shared_ptr<BytesRef> *const curTerm = currentSubs[i]->current;
      if (curTerm != nullptr) {
        constexpr int cmp = term->compareTo(curTerm);
        if (cmp == 0) {
          status = SeekStatus::FOUND;
        } else if (cmp < 0) {
          status = SeekStatus::NOT_FOUND;
        } else {
          status = currentSubs[i]->terms.seekCeil(term);
        }
      } else {
        status = SeekStatus::END;
      }
    } else {
      status = currentSubs[i]->terms.seekCeil(term);
    }

    if (status == SeekStatus::FOUND) {
      top[numTop++] = currentSubs[i];
      current = currentSubs[i]->current = currentSubs[i]->terms.term();
      queue->push_back(currentSubs[i]);
    } else {
      if (status == SeekStatus::NOT_FOUND) {
        currentSubs[i]->current = currentSubs[i]->terms.term();
        assert(currentSubs[i]->current != nullptr);
        queue->push_back(currentSubs[i]);
      } else {
        assert(status == SeekStatus::END);
        // enum exhausted
        currentSubs[i]->current.smartpointerreset();
      }
    }
  }

  if (numTop > 0) {
    // at least one sub had exact match to the requested term
    return SeekStatus::FOUND;
  } else if (queue->size() > 0) {
    // no sub had exact match, but at least one sub found
    // a term after the requested term -- advance to that
    // next term:
    pullTop();
    return SeekStatus::NOT_FOUND;
  } else {
    return SeekStatus::END;
  }
}

void MultiTermsEnum::seekExact(int64_t ord)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t MultiTermsEnum::ord()
{
  throw make_shared<UnsupportedOperationException>();
}

void MultiTermsEnum::pullTop()
{
  // extract all subs from the queue that have the same
  // top term
  assert(numTop == 0);
  numTop = queue->fillTop(top);
  current = top[0]->current;
}

void MultiTermsEnum::pushTop() 
{
  // call next() on each top, and reorder queue
  for (int i = 0; i < numTop; i++) {
    shared_ptr<TermsEnumWithSlice> top = queue->top();
    top->current = top->terms->next();
    if (top->current == nullptr) {
      queue->pop();
    } else {
      queue->updateTop();
    }
  }
  numTop = 0;
}

shared_ptr<BytesRef> MultiTermsEnum::next() 
{
  if (lastSeekExact) {
    // Must seekCeil at this point, so those subs that
    // didn't have the term can find the following term.
    // NOTE: we could save some CPU by only seekCeil the
    // subs that didn't match the last exact seek... but
    // most impls short-circuit if you seekCeil to term
    // they are already on.
    constexpr SeekStatus status = seekCeil(current);
    assert(status == SeekStatus::FOUND);
    lastSeekExact = false;
  }
  lastSeek.reset();

  // restore queue
  pushTop();

  // gather equal top fields
  if (queue->size() > 0) {
    // TODO: we could maybe defer this somewhat costly operation until one of
    // the APIs that needs to see the top is invoked (docFreq, postings, etc.)
    pullTop();
  } else {
    current.reset();
  }

  return current;
}

int MultiTermsEnum::docFreq() 
{
  int sum = 0;
  for (int i = 0; i < numTop; i++) {
    sum += top[i]->terms.docFreq();
  }
  return sum;
}

int64_t MultiTermsEnum::totalTermFreq() 
{
  int64_t sum = 0;
  for (int i = 0; i < numTop; i++) {
    constexpr int64_t v = top[i]->terms.totalTermFreq();
    if (v == -1) {
      return v;
    }
    sum += v;
  }
  return sum;
}

shared_ptr<PostingsEnum>
MultiTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                         int flags) 
{
  shared_ptr<MultiPostingsEnum> docsEnum;

  // Can only reuse if incoming enum is also a MultiDocsEnum
  if (reuse != nullptr &&
      std::dynamic_pointer_cast<MultiPostingsEnum>(reuse) != nullptr) {
    docsEnum = std::static_pointer_cast<MultiPostingsEnum>(reuse);
    // ... and was previously created w/ this MultiTermsEnum:
    if (!docsEnum->canReuse(shared_from_this())) {
      docsEnum =
          make_shared<MultiPostingsEnum>(shared_from_this(), subs.size());
    }
  } else {
    docsEnum = make_shared<MultiPostingsEnum>(shared_from_this(), subs.size());
  }

  int upto = 0;

  ArrayUtil::timSort(top, 0, numTop, INDEX_COMPARATOR);

  for (int i = 0; i < numTop; i++) {

    shared_ptr<TermsEnumWithSlice> *const entry = top[i];

    assert((entry->index < docsEnum->subPostingsEnums.size(),
            to_wstring(entry->index) + L" vs " +
                docsEnum->subPostingsEnums.size() + L"; " + subs.size()));
    shared_ptr<PostingsEnum> *const subPostingsEnum =
        entry->terms->postings(docsEnum->subPostingsEnums[entry->index], flags);
    assert(subPostingsEnum != nullptr);
    docsEnum->subPostingsEnums[entry->index] = subPostingsEnum;
    subDocs[upto]->postingsEnum = subPostingsEnum;
    subDocs[upto]->slice = entry->subSlice;
    upto++;
  }

  return docsEnum->reset(subDocs, upto);
}

MultiTermsEnum::TermsEnumWithSlice::TermsEnumWithSlice(
    int index, shared_ptr<ReaderSlice> subSlice)
    : subSlice(subSlice), index(index)
{
  assert((subSlice->length >= 0, L"length=" + to_wstring(subSlice->length)));
}

void MultiTermsEnum::TermsEnumWithSlice::reset(shared_ptr<TermsEnum> terms,
                                               shared_ptr<BytesRef> term)
{
  this->terms = terms;
  current = term;
}

wstring MultiTermsEnum::TermsEnumWithSlice::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return subSlice->toString() + L":" + terms;
}

MultiTermsEnum::TermMergeQueue::TermMergeQueue(int size)
    : org::apache::lucene::util::PriorityQueue<TermsEnumWithSlice>(size),
      stack(std::deque<int>(size))
{
}

bool MultiTermsEnum::TermMergeQueue::lessThan(
    shared_ptr<TermsEnumWithSlice> termsA,
    shared_ptr<TermsEnumWithSlice> termsB)
{
  return termsA->current->compareTo(termsB->current) < 0;
}

int MultiTermsEnum::TermMergeQueue::fillTop(
    std::deque<std::shared_ptr<TermsEnumWithSlice>> &tops)
{
  constexpr int size = this->size();
  if (size == 0) {
    return 0;
  }
  tops[0] = top();
  int numTop = 1;
  stack[0] = 1;
  int stackLen = 1;

  while (stackLen != 0) {
    constexpr int index = stack[--stackLen];
    constexpr int leftChild = index << 1;
    for (int child = leftChild, end = min(size, leftChild + 1); child <= end;
         ++child) {
      shared_ptr<TermsEnumWithSlice> te = get(child);
      if (te->current->equals(tops[0]->current)) {
        tops[numTop++] = te;
        stack[stackLen++] = child;
      }
    }
  }
  return numTop;
}

shared_ptr<TermsEnumWithSlice> MultiTermsEnum::TermMergeQueue::get(int i)
{
  return any_cast<std::shared_ptr<TermsEnumWithSlice>>(getHeapArray()[i]);
}

wstring MultiTermsEnum::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"MultiTermsEnum(" + Arrays->toString(subs) + L")";
}
} // namespace org::apache::lucene::index