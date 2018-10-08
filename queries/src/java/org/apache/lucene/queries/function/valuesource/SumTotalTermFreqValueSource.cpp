using namespace std;

#include "SumTotalTermFreqValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Terms = org::apache::lucene::index::Terms;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using LongDocValues =
    org::apache::lucene::queries::function::docvalues::LongDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

SumTotalTermFreqValueSource::SumTotalTermFreqValueSource(
    const wstring &indexedField)
    : indexedField(indexedField)
{
}

wstring SumTotalTermFreqValueSource::name() { return L"sumtotaltermfreq"; }

wstring SumTotalTermFreqValueSource::description()
{
  return name() + StringHelper::toString(L'(') + indexedField +
         StringHelper::toString(L')');
}

shared_ptr<FunctionValues> SumTotalTermFreqValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  return std::static_pointer_cast<FunctionValues>(context[shared_from_this()]);
}

void SumTotalTermFreqValueSource::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  int64_t sumTotalTermFreq = 0;
  for (auto readerContext : searcher->getTopReaderContext()->leaves()) {
    shared_ptr<Terms> terms = readerContext->reader()->terms(indexedField);
    if (terms == nullptr) {
      continue;
    }
    int64_t v = terms->getSumTotalTermFreq();
    if (v == -1) {
      sumTotalTermFreq = -1;
      break;
    } else {
      sumTotalTermFreq += v;
    }
  }
  constexpr int64_t ttf = sumTotalTermFreq;
  context.emplace(
      shared_from_this(),
      make_shared<LongDocValuesAnonymousInnerClass>(shared_from_this(), ttf));
}

SumTotalTermFreqValueSource::LongDocValuesAnonymousInnerClass::
    LongDocValuesAnonymousInnerClass(
        shared_ptr<SumTotalTermFreqValueSource> outerInstance, int64_t ttf)
    : org::apache::lucene::queries::function::docvalues::LongDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->ttf = ttf;
}

int64_t
SumTotalTermFreqValueSource::LongDocValuesAnonymousInnerClass::longVal(int doc)
{
  return ttf;
}

int SumTotalTermFreqValueSource::hashCode()
{
  return getClass().hashCode() + indexedField.hashCode();
}

bool SumTotalTermFreqValueSource::equals(any o)
{
  if (this->getClass() != o.type()) {
    return false;
  }
  shared_ptr<SumTotalTermFreqValueSource> other =
      any_cast<std::shared_ptr<SumTotalTermFreqValueSource>>(o);
  return this->indexedField == other->indexedField;
}
} // namespace org::apache::lucene::queries::function::valuesource