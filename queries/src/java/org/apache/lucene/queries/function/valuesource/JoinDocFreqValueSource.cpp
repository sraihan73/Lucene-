using namespace std;

#include "JoinDocFreqValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValues = org::apache::lucene::index::DocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiFields = org::apache::lucene::index::MultiFields;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using IntDocValues =
    org::apache::lucene::queries::function::docvalues::IntDocValues;
using BytesRef = org::apache::lucene::util::BytesRef;
const wstring JoinDocFreqValueSource::NAME = L"joindf";

JoinDocFreqValueSource::JoinDocFreqValueSource(const wstring &field,
                                               const wstring &qfield)
    : FieldCacheSource(field), qfield(qfield)
{
}

wstring JoinDocFreqValueSource::description()
{
  return NAME + L"(" + field + L":(" + qfield + L"))";
}

shared_ptr<FunctionValues> JoinDocFreqValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<BinaryDocValues> *const terms =
      DocValues::getBinary(readerContext->reader(), field);
  shared_ptr<IndexReader> *const top =
      ReaderUtil::getTopLevelContext(readerContext)->reader();
  shared_ptr<Terms> t = MultiFields::getTerms(top, qfield);
  shared_ptr<TermsEnum> *const termsEnum =
      t == nullptr ? TermsEnum::EMPTY : t->begin();

  return make_shared<IntDocValuesAnonymousInnerClass>(shared_from_this(), terms,
                                                      termsEnum);
}

JoinDocFreqValueSource::IntDocValuesAnonymousInnerClass::
    IntDocValuesAnonymousInnerClass(
        shared_ptr<JoinDocFreqValueSource> outerInstance,
        shared_ptr<BinaryDocValues> terms, shared_ptr<TermsEnum> termsEnum)
    : org::apache::lucene::queries::function::docvalues::IntDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->terms = terms;
  this->termsEnum = termsEnum;
  lastDocID = -1;
}

int JoinDocFreqValueSource::IntDocValuesAnonymousInnerClass::intVal(
    int doc) 
{
  if (doc < lastDocID) {
    throw invalid_argument(L"docs were sent out-of-order: lastDocID=" +
                           lastDocID + L" vs docID=" + to_wstring(doc));
  }
  lastDocID = doc;
  int curDocID = terms->docID();
  if (doc > curDocID) {
    curDocID = terms->advance(doc);
  }
  if (doc == curDocID) {
    shared_ptr<BytesRef> term = terms->binaryValue();
    if (termsEnum->seekExact(term)) {
      return termsEnum->docFreq();
    }
  }
  return 0;
}

bool JoinDocFreqValueSource::equals(any o)
{
  if (o.type() != JoinDocFreqValueSource::typeid) {
    return false;
  }
  shared_ptr<JoinDocFreqValueSource> other =
      any_cast<std::shared_ptr<JoinDocFreqValueSource>>(o);
  if (qfield != other->qfield) {
    return false;
  }
  return FieldCacheSource::equals(other);
}

int JoinDocFreqValueSource::hashCode()
{
  return qfield.hashCode() + FieldCacheSource::hashCode();
}
} // namespace org::apache::lucene::queries::function::valuesource