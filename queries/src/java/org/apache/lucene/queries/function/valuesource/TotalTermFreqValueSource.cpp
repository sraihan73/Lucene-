using namespace std;

#include "TotalTermFreqValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using LongDocValues =
    org::apache::lucene::queries::function::docvalues::LongDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using BytesRef = org::apache::lucene::util::BytesRef;

TotalTermFreqValueSource::TotalTermFreqValueSource(
    const wstring &field, const wstring &val, const wstring &indexedField,
    shared_ptr<BytesRef> indexedBytes)
    : field(field), indexedField(indexedField), val(val),
      indexedBytes(indexedBytes)
{
}

wstring TotalTermFreqValueSource::name() { return L"totaltermfreq"; }

wstring TotalTermFreqValueSource::description()
{
  return name() + StringHelper::toString(L'(') + field +
         StringHelper::toString(L',') + val + StringHelper::toString(L')');
}

shared_ptr<FunctionValues> TotalTermFreqValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  return std::static_pointer_cast<FunctionValues>(context[shared_from_this()]);
}

void TotalTermFreqValueSource::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  int64_t totalTermFreq = 0;
  for (auto readerContext : searcher->getTopReaderContext()->leaves()) {
    int64_t val = readerContext->reader()->totalTermFreq(
        make_shared<Term>(indexedField, indexedBytes));
    if (val == -1) {
      totalTermFreq = -1;
      break;
    } else {
      totalTermFreq += val;
    }
  }
  constexpr int64_t ttf = totalTermFreq;
  context.emplace(
      shared_from_this(),
      make_shared<LongDocValuesAnonymousInnerClass>(shared_from_this(), ttf));
}

TotalTermFreqValueSource::LongDocValuesAnonymousInnerClass::
    LongDocValuesAnonymousInnerClass(
        shared_ptr<TotalTermFreqValueSource> outerInstance, int64_t ttf)
    : org::apache::lucene::queries::function::docvalues::LongDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->ttf = ttf;
}

int64_t
TotalTermFreqValueSource::LongDocValuesAnonymousInnerClass::longVal(int doc)
{
  return ttf;
}

int TotalTermFreqValueSource::hashCode()
{
  return getClass().hashCode() + indexedField.hashCode() * 29 +
         indexedBytes->hashCode();
}

bool TotalTermFreqValueSource::equals(any o)
{
  if (this->getClass() != o.type()) {
    return false;
  }
  shared_ptr<TotalTermFreqValueSource> other =
      any_cast<std::shared_ptr<TotalTermFreqValueSource>>(o);
  return this->indexedField == other->indexedField &&
         this->indexedBytes->equals(other->indexedBytes);
}
} // namespace org::apache::lucene::queries::function::valuesource