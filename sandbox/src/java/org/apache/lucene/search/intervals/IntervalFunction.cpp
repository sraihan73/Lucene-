using namespace std;

#include "IntervalFunction.h"

namespace org::apache::lucene::search::intervals
{
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
const shared_ptr<IntervalFunction> IntervalFunction::BLOCK =
    make_shared<SingletonFunctionAnonymousInnerClass>();

IntervalFunction::SingletonFunctionAnonymousInnerClass::
    SingletonFunctionAnonymousInnerClass()
    : SingletonFunction(L"BLOCK")
{
}

shared_ptr<IntervalIterator>
IntervalFunction::SingletonFunctionAnonymousInnerClass::apply(
    deque<std::shared_ptr<IntervalIterator>> &iterators)
{
  return make_shared<BlockIntervalIterator>(iterators);
}

IntervalFunction::BlockIntervalIterator::BlockIntervalIterator(
    deque<std::shared_ptr<IntervalIterator>> &subIterators)
    : ConjunctionIntervalIterator(subIterators)
{
}

int IntervalFunction::BlockIntervalIterator::start() { return start_; }

int IntervalFunction::BlockIntervalIterator::end() { return end_; }

int IntervalFunction::BlockIntervalIterator::nextInterval() 
{
  if (subIterators[0]->nextInterval() == IntervalIterator::NO_MORE_INTERVALS) {
    return IntervalIterator::NO_MORE_INTERVALS;
  }
  int i = 1;
  while (i < subIterators.size()) {
    while (subIterators[i]->start() <= subIterators[i - 1]->end()) {
      if (subIterators[i]->nextInterval() ==
          IntervalIterator::NO_MORE_INTERVALS) {
        return IntervalIterator::NO_MORE_INTERVALS;
      }
    }
    if (subIterators[i]->start() == subIterators[i - 1]->end() + 1) {
      i = i + 1;
    } else {
      if (subIterators[0]->nextInterval() ==
          IntervalIterator::NO_MORE_INTERVALS) {
        return IntervalIterator::NO_MORE_INTERVALS;
      }
      i = 1;
    }
  }
  start_ = subIterators[0]->start();
  end_ = subIterators[subIterators.size() - 1]->end();
  return start_;
}

void IntervalFunction::BlockIntervalIterator::reset() { start_ = end_ = -1; }

const shared_ptr<IntervalFunction> IntervalFunction::ORDERED =
    make_shared<SingletonFunctionAnonymousInnerClass>();

IntervalFunction::SingletonFunctionAnonymousInnerClass::
    SingletonFunctionAnonymousInnerClass()
    : SingletonFunction(L"ORDERED")
{
}

shared_ptr<IntervalIterator>
IntervalFunction::SingletonFunctionAnonymousInnerClass::apply(
    deque<std::shared_ptr<IntervalIterator>> &intervalIterators)
{
  return make_shared<OrderedIntervalIterator>(intervalIterators);
}

IntervalFunction::OrderedIntervalIterator::OrderedIntervalIterator(
    deque<std::shared_ptr<IntervalIterator>> &subIntervals)
    : ConjunctionIntervalIterator(subIntervals)
{
}

int IntervalFunction::OrderedIntervalIterator::start() { return start_; }

int IntervalFunction::OrderedIntervalIterator::end() { return end_; }

int IntervalFunction::OrderedIntervalIterator::nextInterval() 
{
  start_ = end_ = IntervalIterator::NO_MORE_INTERVALS;
  int b = numeric_limits<int>::max();
  i = 1;
  while (true) {
    while (true) {
      if (subIterators[i - 1]->end() >= b) {
        return start_;
      }
      if (i == subIterators.size() ||
          subIterators[i]->start() > subIterators[i - 1]->end()) {
        break;
      }
      do {
        if (subIterators[i]->end() >= b ||
            subIterators[i]->nextInterval() ==
                IntervalIterator::NO_MORE_INTERVALS) {
          return start_;
        }
      } while (subIterators[i]->start() <= subIterators[i - 1]->end());
      i++;
    }
    start_ = subIterators[0]->start();
    end_ = subIterators[subIterators.size() - 1]->end();
    b = subIterators[subIterators.size() - 1]->start();
    i = 1;
    if (subIterators[0]->nextInterval() ==
        IntervalIterator::NO_MORE_INTERVALS) {
      return start_;
    }
  }
}

void IntervalFunction::OrderedIntervalIterator::reset() 
{
  subIterators[0]->nextInterval();
  i = 1;
  start_ = end_ = -1;
}

const shared_ptr<IntervalFunction> IntervalFunction::UNORDERED =
    make_shared<SingletonFunctionAnonymousInnerClass>();

IntervalFunction::SingletonFunctionAnonymousInnerClass::
    SingletonFunctionAnonymousInnerClass()
    : SingletonFunction(L"UNORDERED")
{
}

shared_ptr<IntervalIterator>
IntervalFunction::SingletonFunctionAnonymousInnerClass::apply(
    deque<std::shared_ptr<IntervalIterator>> &intervalIterators)
{
  return make_shared<UnorderedIntervalIterator>(intervalIterators, true);
}

const shared_ptr<IntervalFunction> IntervalFunction::UNORDERED_NO_OVERLAP =
    make_shared<SingletonFunctionAnonymousInnerClass2>();

IntervalFunction::SingletonFunctionAnonymousInnerClass2::
    SingletonFunctionAnonymousInnerClass2()
    : SingletonFunction(L"UNORDERED_NO_OVERLAP")
{
}

shared_ptr<IntervalIterator>
IntervalFunction::SingletonFunctionAnonymousInnerClass2::apply(
    deque<std::shared_ptr<IntervalIterator>> &iterators)
{
  return make_shared<UnorderedIntervalIterator>(iterators, false);
}

IntervalFunction::UnorderedIntervalIterator::UnorderedIntervalIterator(
    deque<std::shared_ptr<IntervalIterator>> &subIterators, bool allowOverlaps)
    : ConjunctionIntervalIterator(subIterators),
      queue(make_shared<PriorityQueue<std::shared_ptr<IntervalIterator>>>(
          subIterators.size())),
      subIterators(
          std::deque<std::shared_ptr<IntervalIterator>>(subIterators.size())),
      allowOverlaps(allowOverlaps)
{
  {protected : bool lessThan(IntervalIterator a, IntervalIterator b){
      return a::start() < b::start() ||
             (a::start() == b::start() && a::end() >= b::end());
}
}; // namespace org::apache::lucene::search::intervals

for (int i = 0; i < subIterators.size(); i++) {
  this->subIterators[i] = subIterators[i];
}
}

int IntervalFunction::UnorderedIntervalIterator::start() { return start_; }

int IntervalFunction::UnorderedIntervalIterator::end() { return end_; }

void IntervalFunction::UnorderedIntervalIterator::updateRightExtreme(
    shared_ptr<IntervalIterator> it)
{
  int itEnd = it->end();
  if (itEnd > queueEnd) {
    queueEnd = itEnd;
  }
}

int IntervalFunction::UnorderedIntervalIterator::nextInterval() throw(
    IOException)
{
  // first, find a matching interval
  while (this->queue->size() == subIterators.size() &&
         queue->top()->start() == start_) {
    shared_ptr<IntervalIterator> it = queue->pop();
    if (it != nullptr &&
        it->nextInterval() != IntervalIterator::NO_MORE_INTERVALS) {
      if (allowOverlaps == false) {
        while (hasOverlaps(it)) {
          if (it->nextInterval() == IntervalIterator::NO_MORE_INTERVALS) {
            return IntervalIterator::NO_MORE_INTERVALS;
          }
        }
      }
      queue->push_back(it);
      updateRightExtreme(it);
    }
  }
  if (this->queue->size() < subIterators.size()) {
    return IntervalIterator::NO_MORE_INTERVALS;
  }
  // then, minimize it
  do {
    start_ = queue->top()->start();
    end_ = queueEnd;
    if (queue->top()->end() == end_) {
      return start_;
    }
    shared_ptr<IntervalIterator> it = queue->pop();
    if (it != nullptr &&
        it->nextInterval() != IntervalIterator::NO_MORE_INTERVALS) {
      if (allowOverlaps == false) {
        while (hasOverlaps(it)) {
          if (it->nextInterval() == IntervalIterator::NO_MORE_INTERVALS) {
            return start_;
          }
        }
      }
      queue->push_back(it);
      updateRightExtreme(it);
    }
  } while (this->queue->size() == subIterators.size() && end_ == queueEnd);
  return start_;
}

void IntervalFunction::UnorderedIntervalIterator::reset() 
{
  queueEnd = start_ = end_ = -1;
  this->queue->clear();
  for (auto it : subIterators) {
    if (it->nextInterval() == NO_MORE_INTERVALS) {
      break;
    }
    if (allowOverlaps == false) {
      while (hasOverlaps(it)) {
        if (it->nextInterval() == NO_MORE_INTERVALS) {
          goto loopBreak;
        }
      }
    }
    queue->push_back(it);
    updateRightExtreme(it);
  loopContinue:;
  }
loopBreak:;
}

bool IntervalFunction::UnorderedIntervalIterator::hasOverlaps(
    shared_ptr<IntervalIterator> candidate)
{
  for (auto it : queue) {
    if (it->start() < candidate->start()) {
      if (it->end() >= candidate->start()) {
        return true;
      }
      continue;
    }
    if (it->start() == candidate->start()) {
      return true;
    }
    if (it->start() <= candidate->end()) {
      return true;
    }
  }
  return false;
}

const shared_ptr<IntervalFunction> IntervalFunction::CONTAINING =
    make_shared<SingletonFunctionAnonymousInnerClass>();

IntervalFunction::SingletonFunctionAnonymousInnerClass::
    SingletonFunctionAnonymousInnerClass()
    : SingletonFunction(L"CONTAINING")
{
}

shared_ptr<IntervalIterator>
IntervalFunction::SingletonFunctionAnonymousInnerClass::apply(
    deque<std::shared_ptr<IntervalIterator>> &iterators)
{
  if (iterators.size() != 2) {
    throw make_shared<IllegalStateException>(
        L"CONTAINING function requires two iterators");
  }
  shared_ptr<IntervalIterator> a = iterators[0];
  shared_ptr<IntervalIterator> b = iterators[1];
  return make_shared<ConjunctionIntervalIteratorAnonymousInnerClass>(
      shared_from_this(), iterators, a, b);
}

IntervalFunction::SingletonFunctionAnonymousInnerClass::
    ConjunctionIntervalIteratorAnonymousInnerClass::
        ConjunctionIntervalIteratorAnonymousInnerClass(
            shared_ptr<SingletonFunctionAnonymousInnerClass> outerInstance,
            deque<std::shared_ptr<IntervalIterator>> &iterators,
            shared_ptr<org::apache::lucene::search::intervals::IntervalIterator>
                a,
            shared_ptr<org::apache::lucene::search::intervals::IntervalIterator>
                b)
    : ConjunctionIntervalIterator(iterators)
{
  this->outerInstance = outerInstance;
  this->a = a;
  this->b = b;
}

int IntervalFunction::SingletonFunctionAnonymousInnerClass::
    ConjunctionIntervalIteratorAnonymousInnerClass::start()
{
  return a->start();
}

int IntervalFunction::SingletonFunctionAnonymousInnerClass::
    ConjunctionIntervalIteratorAnonymousInnerClass::end()
{
  return a->end();
}

int IntervalFunction::SingletonFunctionAnonymousInnerClass::
    ConjunctionIntervalIteratorAnonymousInnerClass::nextInterval() throw(
        IOException)
{
  if (bpos == false) {
    return IntervalIterator::NO_MORE_INTERVALS;
  }
  while (a->nextInterval() != IntervalIterator::NO_MORE_INTERVALS) {
    while (b->start() < a->start() && b->end() < a->end()) {
      if (b->nextInterval() == IntervalIterator::NO_MORE_INTERVALS) {
        return IntervalIterator::NO_MORE_INTERVALS;
      }
    }
    if (a->start() <= b->start() && a->end() >= b->end()) {
      return a->start();
    }
  }
  return IntervalIterator::NO_MORE_INTERVALS;
}

void IntervalFunction::SingletonFunctionAnonymousInnerClass::
    ConjunctionIntervalIteratorAnonymousInnerClass::reset() 
{
  bpos = true;
}

const shared_ptr<IntervalFunction> IntervalFunction::CONTAINED_BY =
    make_shared<SingletonFunctionAnonymousInnerClass2>();

IntervalFunction::SingletonFunctionAnonymousInnerClass2::
    SingletonFunctionAnonymousInnerClass2()
    : SingletonFunction(L"CONTAINED_BY")
{
}

shared_ptr<IntervalIterator>
IntervalFunction::SingletonFunctionAnonymousInnerClass2::apply(
    deque<std::shared_ptr<IntervalIterator>> &iterators)
{
  if (iterators.size() != 2) {
    throw make_shared<IllegalStateException>(
        L"CONTAINED_BY function requires two iterators");
  }
  shared_ptr<IntervalIterator> a = iterators[0];
  shared_ptr<IntervalIterator> b = iterators[1];
  return make_shared<ConjunctionIntervalIteratorAnonymousInnerClass2>(
      shared_from_this(), iterators, a, b);
}

IntervalFunction::SingletonFunctionAnonymousInnerClass2::
    ConjunctionIntervalIteratorAnonymousInnerClass2::
        ConjunctionIntervalIteratorAnonymousInnerClass2(
            shared_ptr<SingletonFunctionAnonymousInnerClass2> outerInstance,
            deque<std::shared_ptr<IntervalIterator>> &iterators,
            shared_ptr<org::apache::lucene::search::intervals::IntervalIterator>
                a,
            shared_ptr<org::apache::lucene::search::intervals::IntervalIterator>
                b)
    : ConjunctionIntervalIterator(iterators)
{
  this->outerInstance = outerInstance;
  this->a = a;
  this->b = b;
}

int IntervalFunction::SingletonFunctionAnonymousInnerClass2::
    ConjunctionIntervalIteratorAnonymousInnerClass2::start()
{
  return a->start();
}

int IntervalFunction::SingletonFunctionAnonymousInnerClass2::
    ConjunctionIntervalIteratorAnonymousInnerClass2::end()
{
  return a->end();
}

int IntervalFunction::SingletonFunctionAnonymousInnerClass2::
    ConjunctionIntervalIteratorAnonymousInnerClass2::nextInterval() throw(
        IOException)
{
  if (bpos == false) {
    return IntervalIterator::NO_MORE_INTERVALS;
  }
  while (a->nextInterval() != IntervalIterator::NO_MORE_INTERVALS) {
    while (b->end() < a->end()) {
      if (b->nextInterval() == IntervalIterator::NO_MORE_INTERVALS) {
        return IntervalIterator::NO_MORE_INTERVALS;
      }
    }
    if (b->start() <= a->start()) {
      return a->start();
    }
  }
  return IntervalIterator::NO_MORE_INTERVALS;
}

void IntervalFunction::SingletonFunctionAnonymousInnerClass2::
    ConjunctionIntervalIteratorAnonymousInnerClass2::reset() 
{
  bpos = true;
}

IntervalFunction::SingletonFunction::SingletonFunction(const wstring &name)
    : name(name)
{
}

int IntervalFunction::SingletonFunction::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

bool IntervalFunction::SingletonFunction::equals(any obj)
{
  return obj == shared_from_this();
}

wstring IntervalFunction::SingletonFunction::toString() { return name; }
}