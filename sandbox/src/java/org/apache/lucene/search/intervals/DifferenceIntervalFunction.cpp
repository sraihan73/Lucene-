using namespace std;

#include "DifferenceIntervalFunction.h"

namespace org::apache::lucene::search::intervals
{

const shared_ptr<DifferenceIntervalFunction>
    DifferenceIntervalFunction::NON_OVERLAPPING =
        make_shared<SingletonFunctionAnonymousInnerClass>();

DifferenceIntervalFunction::SingletonFunctionAnonymousInnerClass::
    SingletonFunctionAnonymousInnerClass()
    : SingletonFunction(L"NON_OVERLAPPING")
{
}

shared_ptr<IntervalIterator>
DifferenceIntervalFunction::SingletonFunctionAnonymousInnerClass::apply(
    shared_ptr<IntervalIterator> minuend,
    shared_ptr<IntervalIterator> subtrahend)
{
  return make_shared<NonOverlappingIterator>(minuend, subtrahend);
}

const shared_ptr<DifferenceIntervalFunction>
    DifferenceIntervalFunction::NOT_CONTAINING =
        make_shared<SingletonFunctionAnonymousInnerClass2>();

DifferenceIntervalFunction::SingletonFunctionAnonymousInnerClass2::
    SingletonFunctionAnonymousInnerClass2()
    : SingletonFunction(L"NOT_CONTAINING")
{
}

shared_ptr<IntervalIterator>
DifferenceIntervalFunction::SingletonFunctionAnonymousInnerClass2::apply(
    shared_ptr<IntervalIterator> minuend,
    shared_ptr<IntervalIterator> subtrahend)
{
  return make_shared<NotContainingIterator>(minuend, subtrahend);
}

const shared_ptr<DifferenceIntervalFunction>
    DifferenceIntervalFunction::NOT_CONTAINED_BY =
        make_shared<SingletonFunctionAnonymousInnerClass3>();

DifferenceIntervalFunction::SingletonFunctionAnonymousInnerClass3::
    SingletonFunctionAnonymousInnerClass3()
    : SingletonFunction(L"NOT_CONTAINED_BY")
{
}

shared_ptr<IntervalIterator>
DifferenceIntervalFunction::SingletonFunctionAnonymousInnerClass3::apply(
    shared_ptr<IntervalIterator> minuend,
    shared_ptr<IntervalIterator> subtrahend)
{
  return make_shared<NotContainedByIterator>(minuend, subtrahend);
}

DifferenceIntervalFunction::RelativeIterator::RelativeIterator(
    shared_ptr<IntervalIterator> a, shared_ptr<IntervalIterator> b)
    : a(a), b(b)
{
}

int DifferenceIntervalFunction::RelativeIterator::docID() { return a->docID(); }

int DifferenceIntervalFunction::RelativeIterator::nextDoc() 
{
  int doc = a->nextDoc();
  reset();
  return doc;
}

int DifferenceIntervalFunction::RelativeIterator::advance(int target) throw(
    IOException)
{
  int doc = a->advance(target);
  reset();
  return doc;
}

int64_t DifferenceIntervalFunction::RelativeIterator::cost()
{
  return a->cost();
}

void DifferenceIntervalFunction::RelativeIterator::reset() 
{
  int doc = a->docID();
  bpos = b->docID() == doc || (b->docID() < doc && b->advance(doc) == doc);
}

int DifferenceIntervalFunction::RelativeIterator::start() { return a->start(); }

int DifferenceIntervalFunction::RelativeIterator::end() { return a->end(); }

float DifferenceIntervalFunction::RelativeIterator::matchCost()
{
  return a->matchCost() + b->matchCost();
}

DifferenceIntervalFunction::NonOverlappingIterator::NonOverlappingIterator(
    shared_ptr<IntervalIterator> minuend,
    shared_ptr<IntervalIterator> subtrahend)
    : RelativeIterator(minuend, subtrahend)
{
}

int DifferenceIntervalFunction::NonOverlappingIterator::nextInterval() throw(
    IOException)
{
  if (bpos == false) {
    return a->nextInterval();
  }
  while (a->nextInterval() != NO_MORE_INTERVALS) {
    while (b->end() < a->start()) {
      if (b->nextInterval() == NO_MORE_INTERVALS) {
        bpos = false;
        return a->start();
      }
    }
    if (b->start() > a->end()) {
      return a->start();
    }
  }
  return NO_MORE_INTERVALS;
}

DifferenceIntervalFunction::NotWithinFunction::NotWithinFunction(int positions)
    : positions(positions)
{
}

bool DifferenceIntervalFunction::NotWithinFunction::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<NotWithinFunction> that =
      any_cast<std::shared_ptr<NotWithinFunction>>(o);
  return positions == that->positions;
}

wstring DifferenceIntervalFunction::NotWithinFunction::toString()
{
  return L"NOTWITHIN/" + to_wstring(positions);
}

int DifferenceIntervalFunction::NotWithinFunction::hashCode()
{
  return Objects::hash(positions);
}

shared_ptr<IntervalIterator>
DifferenceIntervalFunction::NotWithinFunction::apply(
    shared_ptr<IntervalIterator> minuend,
    shared_ptr<IntervalIterator> subtrahend)
{
  shared_ptr<IntervalIterator> notWithin =
      make_shared<IntervalIteratorAnonymousInnerClass>(shared_from_this(),
                                                       subtrahend);
  return NON_OVERLAPPING::apply(minuend, notWithin);
}

DifferenceIntervalFunction::NotWithinFunction::
    IntervalIteratorAnonymousInnerClass::IntervalIteratorAnonymousInnerClass(
        shared_ptr<NotWithinFunction> outerInstance,
        shared_ptr<org::apache::lucene::search::intervals::IntervalIterator>
            subtrahend)
{
  this->outerInstance = outerInstance;
  this->subtrahend = subtrahend;
  positioned = false;
}

int DifferenceIntervalFunction::NotWithinFunction::
    IntervalIteratorAnonymousInnerClass::docID()
{
  return subtrahend->docID();
}

int DifferenceIntervalFunction::NotWithinFunction::
    IntervalIteratorAnonymousInnerClass::nextDoc() 
{
  positioned = false;
  return subtrahend->nextDoc();
}

int DifferenceIntervalFunction::NotWithinFunction::
    IntervalIteratorAnonymousInnerClass::advance(int target) 
{
  positioned = false;
  return subtrahend->advance(target);
}

int64_t DifferenceIntervalFunction::NotWithinFunction::
    IntervalIteratorAnonymousInnerClass::cost()
{
  return subtrahend->cost();
}

int DifferenceIntervalFunction::NotWithinFunction::
    IntervalIteratorAnonymousInnerClass::start()
{
  if (positioned == false) {
    return -1;
  }
  int start = subtrahend->start();
  return max(0, start - outerInstance->positions);
}

int DifferenceIntervalFunction::NotWithinFunction::
    IntervalIteratorAnonymousInnerClass::end()
{
  if (positioned == false) {
    return -1;
  }
  int end = subtrahend->end();
  int newEnd = end + outerInstance->positions;
  if (newEnd < 0) // check for overflow
  {
    return numeric_limits<int>::max();
  }
  return newEnd;
}

int DifferenceIntervalFunction::NotWithinFunction::
    IntervalIteratorAnonymousInnerClass::nextInterval() 
{
  if (positioned == false) {
    positioned = true;
  }
  return subtrahend->nextInterval();
}

float DifferenceIntervalFunction::NotWithinFunction::
    IntervalIteratorAnonymousInnerClass::matchCost()
{
  return subtrahend->matchCost();
}

DifferenceIntervalFunction::NotContainingIterator::NotContainingIterator(
    shared_ptr<IntervalIterator> minuend,
    shared_ptr<IntervalIterator> subtrahend)
    : RelativeIterator(minuend, subtrahend)
{
}

int DifferenceIntervalFunction::NotContainingIterator::nextInterval() throw(
    IOException)
{
  if (bpos == false) {
    return a->nextInterval();
  }
  while (a->nextInterval() != NO_MORE_INTERVALS) {
    while (b->start() < a->start() && b->end() < a->end()) {
      if (b->nextInterval() == NO_MORE_INTERVALS) {
        bpos = false;
        return a->start();
      }
    }
    if (b->start() > a->end()) {
      return a->start();
    }
  }
  return NO_MORE_INTERVALS;
}

DifferenceIntervalFunction::NotContainedByIterator::NotContainedByIterator(
    shared_ptr<IntervalIterator> a, shared_ptr<IntervalIterator> b)
    : RelativeIterator(a, b)
{
}

int DifferenceIntervalFunction::NotContainedByIterator::nextInterval() throw(
    IOException)
{
  if (bpos == false) {
    return a->nextInterval();
  }
  while (a->nextInterval() != NO_MORE_INTERVALS) {
    while (b->end() < a->end()) {
      if (b->nextInterval() == NO_MORE_INTERVALS) {
        return a->start();
      }
    }
    if (a->start() < b->start()) {
      return a->start();
    }
  }
  return NO_MORE_INTERVALS;
}

DifferenceIntervalFunction::SingletonFunction::SingletonFunction(
    const wstring &name)
    : name(name)
{
}

int DifferenceIntervalFunction::SingletonFunction::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

bool DifferenceIntervalFunction::SingletonFunction::equals(any obj)
{
  return obj == shared_from_this();
}

wstring DifferenceIntervalFunction::SingletonFunction::toString()
{
  return name;
}
} // namespace org::apache::lucene::search::intervals