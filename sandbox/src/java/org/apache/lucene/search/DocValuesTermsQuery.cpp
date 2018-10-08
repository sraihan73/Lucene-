using namespace std;

#include "DocValuesTermsQuery.h"

namespace org::apache::lucene::search
{
using DocValues = org::apache::lucene::index::DocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PrefixCodedTerms = org::apache::lucene::index::PrefixCodedTerms;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Term = org::apache::lucene::index::Term;
using TermIterator = org::apache::lucene::index::PrefixCodedTerms::TermIterator;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LongBitSet = org::apache::lucene::util::LongBitSet;

DocValuesTermsQuery::DocValuesTermsQuery(
    const wstring &field,
    shared_ptr<deque<std::shared_ptr<BytesRef>>> terms)
    : field(Objects::requireNonNull(field)), termData(builder::finish()),
      termDataHashCode(termData->hashCode())
{
  Objects::requireNonNull(terms, L"deque of terms must not be null");
  std::deque<std::shared_ptr<BytesRef>> sortedTerms =
      terms->toArray(std::deque<std::shared_ptr<BytesRef>>(terms->size()));
  ArrayUtil::timSort(sortedTerms);
  shared_ptr<PrefixCodedTerms::Builder> builder =
      make_shared<PrefixCodedTerms::Builder>();
  shared_ptr<BytesRef> previous = nullptr;
  for (auto term : sortedTerms) {
    if (term->equals(previous) == false) {
      builder->add(field, term);
    }
    previous = term;
  }
}

DocValuesTermsQuery::DocValuesTermsQuery(const wstring &field,
                                         deque<BytesRef> &terms)
    : DocValuesTermsQuery(field, Arrays::asList(terms))
{
}

DocValuesTermsQuery::DocValuesTermsQuery(const wstring &field,
                                         deque<wstring> &terms)
{
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  shared_from_this()(
      field, make_shared<AbstractListAnonymousInnerClass>(shared_from_this()));
}

DocValuesTermsQuery::AbstractListAnonymousInnerClass::
    AbstractListAnonymousInnerClass(
        shared_ptr<DocValuesTermsQuery> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<BytesRef>
DocValuesTermsQuery::AbstractListAnonymousInnerClass::get(int index)
{
  return make_shared<BytesRef>(terms[index]);
}

int DocValuesTermsQuery::AbstractListAnonymousInnerClass::size()
{
  return terms->length;
}

bool DocValuesTermsQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool DocValuesTermsQuery::equalsTo(shared_ptr<DocValuesTermsQuery> other)
{
  // termData might be heavy to compare so check the hash code first
  return termDataHashCode == other->termDataHashCode &&
         termData->equals(other->termData);
}

int DocValuesTermsQuery::hashCode()
{
  return 31 * classHash() + termDataHashCode;
}

wstring DocValuesTermsQuery::toString(const wstring &defaultField)
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  bool first = true;
  shared_ptr<PrefixCodedTerms::TermIterator> iterator = termData->begin();
  for (shared_ptr<BytesRef> term = iterator->next(); term != nullptr;
       term = iterator->next()) {
    if (!first) {
      builder->append(L' ');
    }
    first = false;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    builder->append((make_shared<Term>(iterator->field(), term))->toString());
  }

  return builder->toString();
}

shared_ptr<Weight>
DocValuesTermsQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                  bool needsScores,
                                  float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

DocValuesTermsQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<DocValuesTermsQuery> outerInstance, float boost)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer>
DocValuesTermsQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<SortedSetDocValues> *const values =
      DocValues::getSortedSet(context->reader(), outerInstance->field);
  shared_ptr<LongBitSet> *const bits =
      make_shared<LongBitSet>(values->getValueCount());
  bool matchesAtLeastOneTerm = false;
  shared_ptr<PrefixCodedTerms::TermIterator> iterator =
      outerInstance->termData->begin();
  for (shared_ptr<BytesRef> term = iterator->next(); term != nullptr;
       term = iterator->next()) {
    constexpr int64_t ord = values->lookupTerm(term);
    if (ord >= 0) {
      matchesAtLeastOneTerm = true;
      bits->set(ord);
    }
  }
  if (matchesAtLeastOneTerm == false) {
    return nullptr;
  }
  return make_shared<ConstantScoreScorer>(
      shared_from_this(), score(),
      make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                       values, bits));
}

DocValuesTermsQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<SortedSetDocValues> values, shared_ptr<LongBitSet> bits)
    : TwoPhaseIterator(values)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->bits = bits;
}

bool DocValuesTermsQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  for (int64_t ord = values->nextOrd();
       ord != SortedSetDocValues::NO_MORE_ORDS; ord = values->nextOrd()) {
    if (bits->get(ord)) {
      return true;
    }
  }
  return false;
}

float DocValuesTermsQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 3; // lookup in a bitset
}

bool DocValuesTermsQuery::ConstantScoreWeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return DocValues::isCacheable(ctx, {outerInstance->field});
}
} // namespace org::apache::lucene::search