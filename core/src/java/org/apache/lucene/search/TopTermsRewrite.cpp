using namespace std;

#include "TopTermsRewrite.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermState = org::apache::lucene::index::TermState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

TopTermsRewrite<B>::TermCollectorAnonymousInnerClass::
    TermCollectorAnonymousInnerClass(
        shared_ptr<TopTermsRewrite<std::shared_ptr<B>>> outerInstance,
        int maxSize,
        shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>> stQueue)
{
  this->outerInstance = outerInstance;
  this->maxSize = maxSize;
  this->stQueue = stQueue;
  maxBoostAtt =
      attributes::addAttribute(MaxNonCompetitiveBoostAttribute::typeid);
  visitedTerms = unordered_map<>();
}

void TopTermsRewrite<B>::TermCollectorAnonymousInnerClass::setNextEnum(
    shared_ptr<TermsEnum> termsEnum)
{
  this->termsEnum = termsEnum;

  assert(compareToLastTerm(nullptr));

  // lazy init the initial ScoreTerm because comparator is not known on ctor:
  if (st == nullptr) {
    st = make_shared<ScoreTerm>(make_shared<TermContext>(topReaderContext));
  }
  boostAtt = termsEnum->attributes()->addAttribute(BoostAttribute::typeid);
}

bool TopTermsRewrite<B>::TermCollectorAnonymousInnerClass::compareToLastTerm(
    shared_ptr<BytesRef> t)
{
  if (lastTerm == nullptr && t != nullptr) {
    lastTerm = make_shared<BytesRefBuilder>();
    lastTerm->append(t);
  } else if (t == nullptr) {
    lastTerm = nullptr;
  } else {
    assert((lastTerm->get()->compareTo(t) < 0,
            L"lastTerm=" + lastTerm + L" t=" + t));
    lastTerm::copyBytes(t);
  }
  return true;
}

bool TopTermsRewrite<B>::TermCollectorAnonymousInnerClass::collect(
    shared_ptr<BytesRef> bytes) 
{
  constexpr float boost = boostAtt::getBoost();

  // make sure within a single seg we always collect
  // terms in order
  assert(compareToLastTerm(bytes));

  // System.out.println("TTR.collect term=" + bytes.utf8ToString() + " boost=" +
  // boost + " ord=" + readerContext.ord);
  // ignore uncompetitive hits
  if (stQueue->size() == maxSize) {
    shared_ptr<ScoreTerm> *const t = stQueue->peek();
    if (boost < t->boost) {
      return true;
    }
    if (boost == t->boost && bytes->compareTo(t->bytes->get()) > 0) {
      return true;
    }
  }
  shared_ptr<ScoreTerm> t = visitedTerms->get(bytes);
  shared_ptr<TermState> *const state = termsEnum::termState();
  assert(state != nullptr);
  if (t != nullptr) {
    // if the term is already in the PQ, only update docFreq of term in PQ
    assert((t->boost == boost,
            L"boost should be equal in all segment TermsEnums"));
    t->termState->register_(state, readerContext::ord, termsEnum::docFreq(),
                            termsEnum::totalTermFreq());
  } else {
    // add new entry in PQ, we must clone the term, else it may get overwritten!
    st::bytes::copyBytes(bytes);
    st->boost = boost;
    visitedTerms::put(st::bytes->get(), st);
    assert(st::termState::docFreq() == 0);
    st::termState::register_(state, readerContext::ord, termsEnum::docFreq(),
                             termsEnum::totalTermFreq());
    stQueue->offer(st);
    // possibly drop entries from queue
    if (stQueue->size() > maxSize) {
      st = stQueue->poll();
      visitedTerms->remove(st::bytes->get());
      st::termState->clear(); // reset the termstate!
    } else {
      st = make_shared<ScoreTerm>(make_shared<TermContext>(topReaderContext));
    }
    assert((stQueue->size() <= maxSize,
            L"the PQ size must be limited to maxSize"));
    // set maxBoostAtt with values to help FuzzyTermsEnum to optimize
    if (stQueue->size() == maxSize) {
      t = stQueue->peek();
      maxBoostAtt::setMaxNonCompetitiveBoost(t->boost);
      maxBoostAtt::setCompetitiveTerm(t->bytes->get());
    }
  }

  return true;
}

TopTermsRewrite<
    B>::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass()
{
}

int TopTermsRewrite<B>::ComparatorAnonymousInnerClass::compare(
    shared_ptr<ScoreTerm> st1, shared_ptr<ScoreTerm> st2)
{
  return st1->bytes->get()->compareTo(st2->bytes->get());
}

TopTermsRewrite<B>::ScoreTerm::ScoreTerm(shared_ptr<TermContext> termState)
    : termState(termState)
{
}

int TopTermsRewrite<B>::ScoreTerm::compareTo(shared_ptr<ScoreTerm> other)
{
  if (this->boost == other->boost) {
    return other->bytes->get()->compareTo(this->bytes->get());
  } else {
    return Float::compare(this->boost, other->boost);
  }
}
} // namespace org::apache::lucene::search