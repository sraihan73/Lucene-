using namespace std;

#include "AssertingSpans.h"

namespace org::apache::lucene::search::spans
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;

AssertingSpans::AssertingSpans(shared_ptr<Spans> in_) : in_(in_) {}

int AssertingSpans::nextStartPosition() 
{
  assert((state != State::DOC_START,
          L"invalid position access, state=" + state + L": " + in_));
  assert((state != State::DOC_FINISHED,
          L"invalid position access, state=" + state + L": " + in_));
  assert((state != State::DOC_UNVERIFIED,
          L"invalid position access, state=" + state + L": " + in_));

  checkCurrentPositions();

  // move to next position
  int prev = in_->startPosition();
  int start = in_->nextStartPosition();
  assert((start >= prev,
          L"invalid startPosition (positions went backwards, previous=" +
              to_wstring(prev) + L"): " + in_));

  // transition state if necessary
  if (start == NO_MORE_POSITIONS) {
    state = State::POS_FINISHED;
  } else {
    state = State::ITERATING;
  }

  // check new positions
  checkCurrentPositions();
  return start;
}

void AssertingSpans::checkCurrentPositions()
{
  int start = in_->startPosition();
  int end = in_->endPosition();

  if (state == State::DOC_START || state == State::DOC_UNVERIFIED ||
      state == State::POS_START) {
    assert((start == -1, L"invalid startPosition (should be -1): " + in_));
    assert((end == -1, L"invalid endPosition (should be -1): " + in_));
  } else if (state == State::POS_FINISHED) {
    assert((start == NO_MORE_POSITIONS,
            L"invalid startPosition (should be NO_MORE_POSITIONS): " + in_));
    assert((end == NO_MORE_POSITIONS,
            L"invalid endPosition (should be NO_MORE_POSITIONS): " + in_));
  } else {
    assert((start >= 0, L"invalid startPosition (negative): " + in_));
    assert((start <= end, L"invalid startPosition (> endPosition): " + in_));
  }
}

int AssertingSpans::startPosition()
{
  checkCurrentPositions();
  return in_->startPosition();
}

int AssertingSpans::endPosition()
{
  checkCurrentPositions();
  return in_->endPosition();
}

int AssertingSpans::width()
{
  assert(state == State::ITERATING);
  constexpr int distance = in_->width();
  assert(distance >= 0);
  return distance;
}

void AssertingSpans::collect(shared_ptr<SpanCollector> collector) throw(
    IOException)
{
  assert((state == State::ITERATING,
          L"collect() called in illegal state: " + state + L": " + in_));
  in_->collect(collector);
}

int AssertingSpans::docID()
{
  int doc = in_->docID();
  assert((doc == this->doc, L"broken docID() impl: docID() = " +
                                to_wstring(doc) +
                                L", but next/advance last returned: " +
                                to_wstring(this->doc) + L": " + in_));
  return doc;
}

int AssertingSpans::nextDoc() 
{
  assert((state != State::DOC_FINISHED,
          L"nextDoc() called after NO_MORE_DOCS: " + in_));
  int nextDoc = in_->nextDoc();
  assert((nextDoc > doc, L"backwards nextDoc from " + to_wstring(doc) +
                             L" to " + to_wstring(nextDoc) + L": " + in_));
  if (nextDoc == DocIdSetIterator::NO_MORE_DOCS) {
    state = State::DOC_FINISHED;
  } else {
    assert((in_->startPosition() == -1,
            L"invalid initial startPosition() [should be -1]: " + in_));
    assert((in_->endPosition() == -1,
            L"invalid initial endPosition() [should be -1]: " + in_));
    state = State::POS_START;
  }
  doc = nextDoc;
  return docID();
}

int AssertingSpans::advance(int target) 
{
  assert((state != State::DOC_FINISHED,
          L"advance() called after NO_MORE_DOCS: " + in_));
  assert((target > doc, L"target must be > docID(), got " + to_wstring(target) +
                            L" <= " + to_wstring(doc) + L": " + in_));
  int advanced = in_->advance(target);
  assert((advanced >= target, L"backwards advance from: " + to_wstring(target) +
                                  L" to: " + to_wstring(advanced) + L": " +
                                  in_));
  if (advanced == DocIdSetIterator::NO_MORE_DOCS) {
    state = State::DOC_FINISHED;
  } else {
    assert((in_->startPosition() == -1,
            L"invalid initial startPosition() [should be -1]: " + in_));
    assert((in_->endPosition() == -1,
            L"invalid initial endPosition() [should be -1]: " + in_));
    state = State::POS_START;
  }
  doc = advanced;
  return docID();
}

wstring AssertingSpans::toString() { return L"Asserting(" + in_ + L")"; }

int64_t AssertingSpans::cost() { return in_->cost(); }

float AssertingSpans::positionsCost()
{
  float cost = in_->positionsCost();
  assert((!isnan(cost), L"positionsCost() should not be NaN"));
  assert((cost > 0, L"positionsCost() must be positive"));
  return cost;
}

shared_ptr<TwoPhaseIterator> AssertingSpans::asTwoPhaseIterator()
{
  shared_ptr<TwoPhaseIterator> *const iterator = in_->asTwoPhaseIterator();
  if (iterator == nullptr) {
    return nullptr;
  }
  return make_shared<AssertingTwoPhaseView>(shared_from_this(), iterator);
}

AssertingSpans::AssertingTwoPhaseView::AssertingTwoPhaseView(
    shared_ptr<AssertingSpans> outerInstance,
    shared_ptr<TwoPhaseIterator> iterator)
    : org::apache::lucene::search::TwoPhaseIterator(
          new AssertingDISI(outerInstance, iterator->approximation())),
      in_(iterator), outerInstance(outerInstance)
{
}

bool AssertingSpans::AssertingTwoPhaseView::matches() 
{
  if (approximation_->docID() == -1 ||
      approximation_->docID() == DocIdSetIterator::NO_MORE_DOCS) {
    throw make_shared<AssertionError>(
        L"matches() should not be called on doc ID " +
        to_wstring(approximation_->docID()));
  }
  if (lastDoc == approximation_->docID()) {
    throw make_shared<AssertionError>(
        L"matches() has been called twice on doc ID " +
        to_wstring(approximation_->docID()));
  }
  lastDoc = approximation_->docID();
  bool v = in_->matches();
  if (v) {
    outerInstance->state = State::POS_START;
  }
  return v;
}

float AssertingSpans::AssertingTwoPhaseView::matchCost()
{
  float cost = in_->matchCost();
  if (isnan(cost)) {
    throw make_shared<AssertionError>(L"matchCost()=" + to_wstring(cost) +
                                      L" should not be NaN on doc ID " +
                                      to_wstring(approximation_->docID()));
  }
  if (cost < 0) {
    throw make_shared<AssertionError>(L"matchCost()=" + to_wstring(cost) +
                                      L" should be non negative on doc ID " +
                                      to_wstring(approximation_->docID()));
  }
  return cost;
}

AssertingSpans::AssertingDISI::AssertingDISI(
    shared_ptr<AssertingSpans> outerInstance, shared_ptr<DocIdSetIterator> in_)
    : in_(in_), outerInstance(outerInstance)
{
}

int AssertingSpans::AssertingDISI::docID()
{
  assert(in_->docID() == outerInstance->docID());
  return in_->docID();
}

int AssertingSpans::AssertingDISI::nextDoc() 
{
  assert((outerInstance->state != State::DOC_FINISHED,
          L"nextDoc() called after NO_MORE_DOCS: " + in_));
  int nextDoc = in_->nextDoc();
  assert((nextDoc > outerInstance->doc,
          L"backwards nextDoc from " + to_wstring(outerInstance->doc) +
              L" to " + to_wstring(nextDoc) + L": " + in_));
  if (nextDoc == DocIdSetIterator::NO_MORE_DOCS) {
    outerInstance->state = State::DOC_FINISHED;
  } else {
    outerInstance->state = State::DOC_UNVERIFIED;
  }
  outerInstance->doc = nextDoc;
  return docID();
}

int AssertingSpans::AssertingDISI::advance(int target) 
{
  assert((outerInstance->state != State::DOC_FINISHED,
          L"advance() called after NO_MORE_DOCS: " + in_));
  assert((target > outerInstance->doc,
          L"target must be > docID(), got " + to_wstring(target) + L" <= " +
              to_wstring(outerInstance->doc) + L": " + in_));
  int advanced = in_->advance(target);
  assert((advanced >= target, L"backwards advance from: " + to_wstring(target) +
                                  L" to: " + to_wstring(advanced) + L": " +
                                  in_));
  if (advanced == DocIdSetIterator::NO_MORE_DOCS) {
    outerInstance->state = State::DOC_FINISHED;
  } else {
    outerInstance->state = State::DOC_UNVERIFIED;
  }
  outerInstance->doc = advanced;
  return docID();
}

int64_t AssertingSpans::AssertingDISI::cost() { return in_->cost(); }
} // namespace org::apache::lucene::search::spans