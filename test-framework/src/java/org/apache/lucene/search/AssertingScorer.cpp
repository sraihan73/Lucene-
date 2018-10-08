using namespace std;

#include "AssertingScorer.h"

namespace org::apache::lucene::search
{

shared_ptr<Scorer> AssertingScorer::wrap(shared_ptr<Random> random,
                                         shared_ptr<Scorer> other,
                                         bool canScore)
{
  if (other == nullptr) {
    return nullptr;
  }
  return make_shared<AssertingScorer>(random, other, canScore);
}

AssertingScorer::AssertingScorer(shared_ptr<Random> random,
                                 shared_ptr<Scorer> in_, bool needsScores)
    : Scorer(in_->weight), random(random), in_(in_), needsScores(needsScores)
{
  this->doc = in_->docID();
}

shared_ptr<Scorer> AssertingScorer::getIn() { return in_; }

bool AssertingScorer::iterating()
{
  // we cannot assert that state == ITERATING because of CachingScorerWrapper
  switch (docID()) {
  case -1:
  case DocIdSetIterator::NO_MORE_DOCS:
    return false;
  default:
    return state !=
           IteratorState::APPROXIMATING; // Matches must be confirmed before
                                         // calling freq() or score()
  }
}

float AssertingScorer::score() 
{
  assert(needsScores);
  assert(iterating());
  constexpr float score = in_->score();
  assert((!isnan(score), L"NaN score for in=" + in_));
  return score;
}

shared_ptr<deque<std::shared_ptr<ChildScorer>>>
AssertingScorer::getChildren()
{
  // We cannot hide that we hold a single child, else
  // collectors (e.g. ToParentBlockJoinCollector) that
  // need to walk the scorer tree will miss/skip the
  // Scorer we wrap:
  return Collections::singletonList(make_shared<ChildScorer>(in_, L"SHOULD"));
}

int AssertingScorer::docID() { return in_->docID(); }

wstring AssertingScorer::toString() { return L"AssertingScorer(" + in_ + L")"; }

shared_ptr<DocIdSetIterator> AssertingScorer::iterator()
{
  shared_ptr<DocIdSetIterator> *const in_ = this->in_->begin();
  assert(in_ != nullptr);
  return make_shared<DocIdSetIteratorAnonymousInnerClass>(shared_from_this(),
                                                          in_);
}

AssertingScorer::DocIdSetIteratorAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass(
        shared_ptr<AssertingScorer> outerInstance,
        shared_ptr<org::apache::lucene::search::DocIdSetIterator> in_)
{
  this->outerInstance = outerInstance;
  this->in_ = in_;
}

int AssertingScorer::DocIdSetIteratorAnonymousInnerClass::docID()
{
  assert(outerInstance->in_->docID() == in_->docID());
  return in_->docID();
}

int AssertingScorer::DocIdSetIteratorAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  assert((outerInstance->state != IteratorState::FINISHED,
          L"nextDoc() called after NO_MORE_DOCS"));
  int nextDoc = in_->nextDoc();
  assert((nextDoc > outerInstance->doc,
          L"backwards nextDoc from " + to_wstring(outerInstance->doc) +
              L" to " + to_wstring(nextDoc) + L" " + in_));
  if (nextDoc == DocIdSetIterator::NO_MORE_DOCS) {
    outerInstance->state = IteratorState::FINISHED;
  } else {
    outerInstance->state = IteratorState::ITERATING;
  }
  assert(in_->docID() == nextDoc);
  assert(outerInstance->in_->docID() == in_->docID());
  return outerInstance->doc = nextDoc;
}

int AssertingScorer::DocIdSetIteratorAnonymousInnerClass::advance(
    int target) 
{
  assert((outerInstance->state != IteratorState::FINISHED,
          L"advance() called after NO_MORE_DOCS"));
  assert((target > outerInstance->doc, L"target must be > docID(), got " +
                                           to_wstring(target) + L" <= " +
                                           to_wstring(outerInstance->doc)));
  int advanced = in_->advance(target);
  assert((advanced >= target, L"backwards advance from: " + to_wstring(target) +
                                  L" to: " + to_wstring(advanced)));
  if (advanced == DocIdSetIterator::NO_MORE_DOCS) {
    outerInstance->state = IteratorState::FINISHED;
  } else {
    outerInstance->state = IteratorState::ITERATING;
  }
  assert(in_->docID() == advanced);
  assert(outerInstance->in_->docID() == in_->docID());
  return outerInstance->doc = advanced;
}

int64_t AssertingScorer::DocIdSetIteratorAnonymousInnerClass::cost()
{
  return in_->cost();
}

shared_ptr<TwoPhaseIterator> AssertingScorer::twoPhaseIterator()
{
  shared_ptr<TwoPhaseIterator> *const in_ = this->in_->twoPhaseIterator();
  if (in_ == nullptr) {
    return nullptr;
  }
  shared_ptr<DocIdSetIterator> *const inApproximation = in_->approximation();
  assert(inApproximation->docID() == doc);
  shared_ptr<DocIdSetIterator> *const assertingApproximation =
      make_shared<DocIdSetIteratorAnonymousInnerClass2>(shared_from_this(),
                                                        inApproximation);
  return make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                          in_, inApproximation);
}

AssertingScorer::DocIdSetIteratorAnonymousInnerClass2::
    DocIdSetIteratorAnonymousInnerClass2(
        shared_ptr<AssertingScorer> outerInstance,
        shared_ptr<org::apache::lucene::search::DocIdSetIterator>
            inApproximation)
{
  this->outerInstance = outerInstance;
  this->inApproximation = inApproximation;
}

int AssertingScorer::DocIdSetIteratorAnonymousInnerClass2::docID()
{
  return inApproximation->docID();
}

int AssertingScorer::DocIdSetIteratorAnonymousInnerClass2::nextDoc() throw(
    IOException)
{
  assert((outerInstance->state != IteratorState::FINISHED,
          L"advance() called after NO_MORE_DOCS"));
  constexpr int nextDoc = inApproximation->nextDoc();
  assert((nextDoc > outerInstance->doc, L"backwards advance from: " +
                                            to_wstring(outerInstance->doc) +
                                            L" to: " + to_wstring(nextDoc)));
  if (nextDoc == NO_MORE_DOCS) {
    outerInstance->state = IteratorState::FINISHED;
  } else {
    outerInstance->state = IteratorState::APPROXIMATING;
  }
  assert(inApproximation->docID() == nextDoc);
  return outerInstance->doc = nextDoc;
}

int AssertingScorer::DocIdSetIteratorAnonymousInnerClass2::advance(
    int target) 
{
  assert((outerInstance->state != IteratorState::FINISHED,
          L"advance() called after NO_MORE_DOCS"));
  assert((target > outerInstance->doc, L"target must be > docID(), got " +
                                           to_wstring(target) + L" <= " +
                                           to_wstring(outerInstance->doc)));
  constexpr int advanced = inApproximation->advance(target);
  assert((advanced >= target, L"backwards advance from: " + to_wstring(target) +
                                  L" to: " + to_wstring(advanced)));
  if (advanced == NO_MORE_DOCS) {
    outerInstance->state = IteratorState::FINISHED;
  } else {
    outerInstance->state = IteratorState::APPROXIMATING;
  }
  assert(inApproximation->docID() == advanced);
  return outerInstance->doc = advanced;
}

int64_t AssertingScorer::DocIdSetIteratorAnonymousInnerClass2::cost()
{
  return inApproximation->cost();
}

AssertingScorer::TwoPhaseIteratorAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<AssertingScorer> outerInstance,
        shared_ptr<org::apache::lucene::search::TwoPhaseIterator> in_,
        shared_ptr<org::apache::lucene::search::DocIdSetIterator>
            inApproximation)
    : TwoPhaseIterator(assertingApproximation)
{
  this->outerInstance = outerInstance;
  this->in_ = in_;
  this->inApproximation = inApproximation;
}

bool AssertingScorer::TwoPhaseIteratorAnonymousInnerClass::matches() throw(
    IOException)
{
  assert(outerInstance->state == IteratorState::APPROXIMATING);
  constexpr bool matches = in_->matches();
  if (matches) {
    assert((outerInstance->in_->begin().docID() == inApproximation->docID(),
            L"Approximation and scorer don't advance synchronously"));
    outerInstance->doc = inApproximation->docID();
    outerInstance->state = IteratorState::ITERATING;
  }
  return matches;
}

float AssertingScorer::TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  float matchCost = in_->matchCost();
  assert(!isnan(matchCost));
  assert(matchCost >= 0);
  return matchCost;
}

wstring AssertingScorer::TwoPhaseIteratorAnonymousInnerClass::toString()
{
  return L"AssertingScorer@asTwoPhaseIterator(" + in_ + L")";
}
} // namespace org::apache::lucene::search