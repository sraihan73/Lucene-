using namespace std;

#include "DocValuesRewriteMethod.h"

namespace org::apache::lucene::search
{
using DocValues = org::apache::lucene::index::DocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using LongBitSet = org::apache::lucene::util::LongBitSet;

shared_ptr<Query>
DocValuesRewriteMethod::rewrite(shared_ptr<IndexReader> reader,
                                shared_ptr<MultiTermQuery> query)
{
  return make_shared<ConstantScoreQuery>(
      make_shared<MultiTermQueryDocValuesWrapper>(query));
}

DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    MultiTermQueryDocValuesWrapper(shared_ptr<MultiTermQuery> query)
    : query(query)
{
}

wstring DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::toString(
    const wstring &field)
{
  // query.toString should be ok for the filter, too, if the query boost is 1.0f
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return query->toString(field);
}

bool DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::equals(
    any const other)
{
  return sameClassAs(other) &&
         query->equals(
             (any_cast<std::shared_ptr<MultiTermQueryDocValuesWrapper>>(other))
                 .query);
}

int DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::hashCode()
{
  return 31 * classHash() + query->hashCode();
}

wstring DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::getField()
{
  return query->getField();
}

shared_ptr<Weight>
DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::
        ConstantScoreWeightAnonymousInnerClass(
            shared_ptr<MultiTermQueryDocValuesWrapper> outerInstance,
            float boost)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Matches> DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::matches(
        shared_ptr<LeafReaderContext> context, int doc) 
{
  shared_ptr<SortedSetDocValues> *const fcsi =
      DocValues::getSortedSet(context->reader(), outerInstance->query->field);
  return Matches::forField(outerInstance->query->field, [&]() {
    DisjunctionMatchesIterator::fromTermsEnum(
        context, doc, outerInstance->query->field, getTermsEnum(fcsi));
  });
}

shared_ptr<TermsEnum> DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::getTermsEnum(
        shared_ptr<SortedSetDocValues> fcsi) 
{
  return outerInstance->query->getTermsEnum(
      make_shared<TermsAnonymousInnerClass>(shared_from_this(), fcsi));
}

DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::TermsAnonymousInnerClass::
        TermsAnonymousInnerClass(
            shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
            shared_ptr<SortedSetDocValues> fcsi)
{
  this->outerInstance = outerInstance;
  this->fcsi = fcsi;
}

shared_ptr<TermsEnum> DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::TermsAnonymousInnerClass::
        iterator() 
{
  return fcsi->termsEnum();
}

int64_t DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::TermsAnonymousInnerClass::
        getSumTotalTermFreq()
{
  return -1;
}

int64_t DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::TermsAnonymousInnerClass::
        getSumDocFreq()
{
  return -1;
}

int DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::TermsAnonymousInnerClass::
        getDocCount()
{
  return -1;
}

int64_t DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::TermsAnonymousInnerClass::size()
{
  return -1;
}

bool DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::TermsAnonymousInnerClass::hasFreqs()
{
  return false;
}

bool DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::TermsAnonymousInnerClass::
        hasOffsets()
{
  return false;
}

bool DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::TermsAnonymousInnerClass::
        hasPositions()
{
  return false;
}

bool DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::TermsAnonymousInnerClass::
        hasPayloads()
{
  return false;
}

shared_ptr<Scorer> DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::scorer(
        shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<SortedSetDocValues> *const fcsi =
      DocValues::getSortedSet(context->reader(), outerInstance->query->field);
  shared_ptr<TermsEnum> termsEnum = getTermsEnum(fcsi);
  assert(termsEnum != nullptr);
  if (termsEnum->next() == nullptr) {
    // no matching terms
    return nullptr;
  }
  // fill into a bitset
  // Cannot use FixedBitSet because we require long index (ord):
  shared_ptr<LongBitSet> *const termSet =
      make_shared<LongBitSet>(fcsi->getValueCount());
  do {
    int64_t ord = termsEnum->ord();
    if (ord >= 0) {
      termSet->set(ord);
    }
  } while (termsEnum->next() != nullptr);

  return make_shared<ConstantScoreScorer>(
      shared_from_this(), score(),
      make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(), fcsi,
                                                       termSet, ord));
}

DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::
        TwoPhaseIteratorAnonymousInnerClass::
            TwoPhaseIteratorAnonymousInnerClass(
                shared_ptr<ConstantScoreWeightAnonymousInnerClass>
                    outerInstance,
                shared_ptr<SortedSetDocValues> fcsi,
                shared_ptr<LongBitSet> termSet, int64_t ord)
    : TwoPhaseIterator(fcsi)
{
  this->outerInstance = outerInstance;
  this->fcsi = fcsi;
  this->termSet = termSet;
  this->ord = ord;
}

bool DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::
        TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  for (int64_t ord = fcsi->nextOrd(); ord != SortedSetDocValues::NO_MORE_ORDS;
       ord = fcsi->nextOrd()) {
    if (termSet->get(ord)) {
      return true;
    }
  }
  return false;
}

float DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::
        TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 3; // lookup in a bitset
}

bool DocValuesRewriteMethod::MultiTermQueryDocValuesWrapper::
    ConstantScoreWeightAnonymousInnerClass::isCacheable(
        shared_ptr<LeafReaderContext> ctx)
{
  return DocValues::isCacheable(ctx, {outerInstance->query->field});
}

bool DocValuesRewriteMethod::equals(any other)
{
  return other != nullptr && getClass() == other.type();
}

int DocValuesRewriteMethod::hashCode() { return 641; }
} // namespace org::apache::lucene::search