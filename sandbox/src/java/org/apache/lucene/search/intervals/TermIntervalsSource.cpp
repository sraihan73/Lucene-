using namespace std;

#include "TermIntervalsSource.h"

namespace org::apache::lucene::search::intervals
{
using Lucene50PostingsFormat =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsFormat;
using Lucene50PostingsReader =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using BytesRef = org::apache::lucene::util::BytesRef;

TermIntervalsSource::TermIntervalsSource(shared_ptr<BytesRef> term) : term(term)
{
}

shared_ptr<IntervalIterator> TermIntervalsSource::intervals(
    const wstring &field, shared_ptr<LeafReaderContext> ctx) 
{
  shared_ptr<Terms> terms = ctx->reader()->terms(field);
  if (terms == nullptr) {
    return nullptr;
  }
  if (terms->hasPositions() == false) {
    throw invalid_argument(L"Cannot create an IntervalIterator over field " +
                           field + L" because it has no indexed positions");
  }
  shared_ptr<TermsEnum> te = terms->begin();
  if (te->seekExact(term) == false) {
    return nullptr;
  }
  shared_ptr<PostingsEnum> pe = te->postings(nullptr, PostingsEnum::POSITIONS);
  float cost = termPositionsCost(te);
  return make_shared<IntervalIteratorAnonymousInnerClass>(shared_from_this(),
                                                          pe, cost);
}

TermIntervalsSource::IntervalIteratorAnonymousInnerClass::
    IntervalIteratorAnonymousInnerClass(
        shared_ptr<TermIntervalsSource> outerInstance,
        shared_ptr<PostingsEnum> pe, float cost)
{
  this->outerInstance = outerInstance;
  this->pe = pe;
  this->cost = cost;
  pos = -1, upto;
}

int TermIntervalsSource::IntervalIteratorAnonymousInnerClass::docID()
{
  return pe->docID();
}

int TermIntervalsSource::IntervalIteratorAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  int doc = pe->nextDoc();
  reset();
  return doc;
}

int TermIntervalsSource::IntervalIteratorAnonymousInnerClass::advance(
    int target) 
{
  int doc = pe->advance(target);
  reset();
  return doc;
}

int64_t TermIntervalsSource::IntervalIteratorAnonymousInnerClass::cost()
{
  return pe->cost();
}

int TermIntervalsSource::IntervalIteratorAnonymousInnerClass::start()
{
  return pos;
}

int TermIntervalsSource::IntervalIteratorAnonymousInnerClass::end()
{
  return pos;
}

int TermIntervalsSource::IntervalIteratorAnonymousInnerClass::
    nextInterval() 
{
  if (upto <= 0) {
    return pos = NO_MORE_INTERVALS;
  }
  upto--;
  return pos = pe->nextPosition();
}

float TermIntervalsSource::IntervalIteratorAnonymousInnerClass::matchCost()
{
  return cost;
}

void TermIntervalsSource::IntervalIteratorAnonymousInnerClass::reset() throw(
    IOException)
{
  if (pe->docID() == DocIdSetIterator::NO_MORE_DOCS) {
    upto = -1;
    pos = NO_MORE_INTERVALS;
  } else {
    upto = pe->freq();
    pos = -1;
  }
}

wstring TermIntervalsSource::IntervalIteratorAnonymousInnerClass::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return outerInstance->term->utf8ToString() + L":" +
         outerInstance->super->toString();
}

int TermIntervalsSource::hashCode() { return Objects::hash(term); }

bool TermIntervalsSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<TermIntervalsSource> that =
      any_cast<std::shared_ptr<TermIntervalsSource>>(o);
  return Objects::equals(term, that->term);
}

wstring TermIntervalsSource::toString() { return term->utf8ToString(); }

void TermIntervalsSource::extractTerms(
    const wstring &field, shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  terms->add(make_shared<Term>(field, term));
}

float TermIntervalsSource::termPositionsCost(
    shared_ptr<TermsEnum> termsEnum) 
{
  // TODO: When intervals move to core, refactor to use the copy of this in
  // PhraseQuery
  int docFreq = termsEnum->docFreq();
  assert(docFreq > 0);
  int64_t totalTermFreq = termsEnum->totalTermFreq();
  float expOccurrencesInMatchingDoc =
      totalTermFreq / static_cast<float>(docFreq);
  return TERM_POSNS_SEEK_OPS_PER_DOC +
         expOccurrencesInMatchingDoc * TERM_OPS_PER_POS;
}
} // namespace org::apache::lucene::search::intervals