using namespace std;

#include "DocValuesNumbersQuery.h"

namespace org::apache::lucene::search
{
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;

DocValuesNumbersQuery::DocValuesNumbersQuery(const wstring &field,
                                             std::deque<int64_t> &numbers)
    : field(Objects::requireNonNull(field)),
      numbers(make_shared<LongHashSet>(numbers))
{
}

DocValuesNumbersQuery::DocValuesNumbersQuery(
    const wstring &field, shared_ptr<deque<int64_t>> numbers)
    : field(Objects::requireNonNull(field)),
      numbers(make_shared<LongHashSet>(
          numbers->stream()
              .mapToLong(optional<int64_t>::longValue)
              .toArray()))
{
}

DocValuesNumbersQuery::DocValuesNumbersQuery(const wstring &field,
                                             deque<int64_t> &numbers)
    : DocValuesNumbersQuery(field, new HashSet<long>(Arrays::asList(numbers)))
{
}

bool DocValuesNumbersQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool DocValuesNumbersQuery::equalsTo(shared_ptr<DocValuesNumbersQuery> other)
{
  return field == other->field && numbers->equals(other->numbers);
}

int DocValuesNumbersQuery::hashCode()
{
  return 31 * classHash() + Objects::hash(field, numbers);
}

wstring DocValuesNumbersQuery::getField() { return field; }

shared_ptr<Set<int64_t>> DocValuesNumbersQuery::getNumbers()
{
  return numbers;
}

wstring DocValuesNumbersQuery::toString(const wstring &defaultField)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return (make_shared<StringBuilder>())
      ->append(field)
      ->append(L": ")
      ->append(numbers->toString())
      ->toString();
}

shared_ptr<Weight>
DocValuesNumbersQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                    bool needsScores,
                                    float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

DocValuesNumbersQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<DocValuesNumbersQuery> outerInstance, float boost)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer>
DocValuesNumbersQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<SortedNumericDocValues> *const values =
      DocValues::getSortedNumeric(context->reader(), outerInstance->field);
  return make_shared<ConstantScoreScorer>(
      shared_from_this(), score(),
      make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                       values));
}

DocValuesNumbersQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<SortedNumericDocValues> values)
    : TwoPhaseIterator(values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

bool DocValuesNumbersQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  int count = values->docValueCount();
  for (int i = 0; i < count; i++) {
    if (outerInstance->outerInstance.numbers->contains(values->nextValue())) {
      return true;
    }
  }
  return false;
}

float DocValuesNumbersQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 5; // lookup in the set
}

bool DocValuesNumbersQuery::ConstantScoreWeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}
} // namespace org::apache::lucene::search