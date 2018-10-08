using namespace std;

#include "FilterSpans.h"

namespace org::apache::lucene::search::spans
{
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;

FilterSpans::FilterSpans(shared_ptr<Spans> in_)
    : in_(Objects::requireNonNull(in_))
{
}

int FilterSpans::nextDoc() 
{
  while (true) {
    int doc = in_->nextDoc();
    if (doc == NO_MORE_DOCS) {
      return NO_MORE_DOCS;
    } else if (twoPhaseCurrentDocMatches()) {
      return doc;
    }
  }
}

int FilterSpans::advance(int target) 
{
  int doc = in_->advance(target);
  while (doc != NO_MORE_DOCS) {
    if (twoPhaseCurrentDocMatches()) {
      break;
    }
    doc = in_->nextDoc();
  }

  return doc;
}

int FilterSpans::docID() { return in_->docID(); }

int FilterSpans::nextStartPosition() 
{
  if (atFirstInCurrentDoc) {
    atFirstInCurrentDoc = false;
    return startPos;
  }

  for (;;) {
    startPos = in_->nextStartPosition();
    if (startPos == NO_MORE_POSITIONS) {
      return NO_MORE_POSITIONS;
    }
    switch (accept(in_)) {
    case YES:
      return startPos;
    case NO:
      break;
    case NO_MORE_IN_CURRENT_DOC:
      return startPos =
                 NO_MORE_POSITIONS; // startPos ahead for the current doc.
    }
  }
}

int FilterSpans::startPosition() { return atFirstInCurrentDoc ? -1 : startPos; }

int FilterSpans::endPosition()
{
  return atFirstInCurrentDoc
             ? -1
             : (startPos != NO_MORE_POSITIONS) ? in_->endPosition()
                                               : NO_MORE_POSITIONS;
}

int FilterSpans::width() { return in_->width(); }

void FilterSpans::collect(shared_ptr<SpanCollector> collector) throw(
    IOException)
{
  in_->collect(collector);
}

int64_t FilterSpans::cost() { return in_->cost(); }

wstring FilterSpans::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"Filter(" + in_->toString() + L")";
}

shared_ptr<TwoPhaseIterator> FilterSpans::asTwoPhaseIterator()
{
  shared_ptr<TwoPhaseIterator> inner = in_->asTwoPhaseIterator();
  if (inner != nullptr) {
    // wrapped instance has an approximation
    return make_shared<TwoPhaseIteratorAnonymousInnerClass>(
        shared_from_this(), inner->approximation(), inner);
  } else {
    // wrapped instance has no approximation, but
    // we can still defer matching until absolutely needed.
    return make_shared<TwoPhaseIteratorAnonymousInnerClass2>(shared_from_this(),
                                                             in_);
  }
}

FilterSpans::TwoPhaseIteratorAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<FilterSpans> outerInstance,
        shared_ptr<org::apache::lucene::search::DocIdSetIterator> approximation,
        shared_ptr<TwoPhaseIterator> inner)
    : org::apache::lucene::search::TwoPhaseIterator(approximation)
{
  this->outerInstance = outerInstance;
  this->inner = inner;
}

bool FilterSpans::TwoPhaseIteratorAnonymousInnerClass::matches() throw(
    IOException)
{
  return inner->matches() && outerInstance->twoPhaseCurrentDocMatches();
}

float FilterSpans::TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return inner->matchCost(); // underestimate
}

wstring FilterSpans::TwoPhaseIteratorAnonymousInnerClass::toString()
{
  return L"FilterSpans@asTwoPhaseIterator(inner=" + inner + L", in=" +
         outerInstance->in_ + L")";
}

FilterSpans::TwoPhaseIteratorAnonymousInnerClass2::
    TwoPhaseIteratorAnonymousInnerClass2(
        shared_ptr<FilterSpans> outerInstance,
        shared_ptr<org::apache::lucene::search::spans::Spans> in_)
    : org::apache::lucene::search::TwoPhaseIterator(in_)
{
  this->outerInstance = outerInstance;
}

bool FilterSpans::TwoPhaseIteratorAnonymousInnerClass2::matches() throw(
    IOException)
{
  return outerInstance->twoPhaseCurrentDocMatches();
}

float FilterSpans::TwoPhaseIteratorAnonymousInnerClass2::matchCost()
{
  return outerInstance->in_->positionsCost(); // overestimate
}

wstring FilterSpans::TwoPhaseIteratorAnonymousInnerClass2::toString()
{
  return L"FilterSpans@asTwoPhaseIterator(in=" + outerInstance->in_ + L")";
}

float FilterSpans::positionsCost()
{
  throw make_shared<UnsupportedOperationException>(); // asTwoPhaseIterator
                                                      // never returns null
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") private final bool
// twoPhaseCurrentDocMatches() throws java.io.IOException
bool FilterSpans::twoPhaseCurrentDocMatches() 
{
  atFirstInCurrentDoc = false;
  startPos = in_->nextStartPosition();
  assert(startPos != NO_MORE_POSITIONS);
  for (;;) {
    switch (accept(in_)) {
    case YES:
      atFirstInCurrentDoc = true;
      return true;
    case NO:
      startPos = in_->nextStartPosition();
      if (startPos != NO_MORE_POSITIONS) {
        break;
      }
      // else fallthrough
    case NO_MORE_IN_CURRENT_DOC:
      startPos = -1;
      return false;
    }
  }
}
} // namespace org::apache::lucene::search::spans