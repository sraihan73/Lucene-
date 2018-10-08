using namespace std;

#include "DocTermsIndexDocValues.h"

namespace org::apache::lucene::queries::function::docvalues
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using ValueSourceScorer =
    org::apache::lucene::queries::function::ValueSourceScorer;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueStr = org::apache::lucene::util::mutable_::MutableValueStr;

DocTermsIndexDocValues::DocTermsIndexDocValues(
    shared_ptr<ValueSource> vs, shared_ptr<LeafReaderContext> context,
    const wstring &field) 
    : DocTermsIndexDocValues(field, vs, open(context, field))
{
}

DocTermsIndexDocValues::DocTermsIndexDocValues(
    const wstring &field, shared_ptr<ValueSource> vs,
    shared_ptr<SortedDocValues> termsIndex)
    : termsIndex(termsIndex), vs(vs), field(field)
{
}

int DocTermsIndexDocValues::getOrdForDoc(int doc) 
{
  if (doc < lastDocID) {
    throw invalid_argument(L"docs were sent out-of-order: lastDocID=" +
                           to_wstring(lastDocID) + L" vs docID=" +
                           to_wstring(doc));
  }
  lastDocID = doc;
  int curDocID = termsIndex->docID();
  if (doc > curDocID) {
    curDocID = termsIndex->advance(doc);
  }
  if (doc == curDocID) {
    return termsIndex->ordValue();
  } else {
    return -1;
  }
}

bool DocTermsIndexDocValues::exists(int doc) 
{
  return getOrdForDoc(doc) >= 0;
}

int DocTermsIndexDocValues::ordVal(int doc) 
{
  return getOrdForDoc(doc);
}

int DocTermsIndexDocValues::numOrd() { return termsIndex->getValueCount(); }

bool DocTermsIndexDocValues::bytesVal(
    int doc, shared_ptr<BytesRefBuilder> target) 
{
  target->clear();
  if (getOrdForDoc(doc) == -1) {
    return false;
  } else {
    target->copyBytes(termsIndex->binaryValue());
    return true;
  }
}

wstring DocTermsIndexDocValues::strVal(int doc) 
{
  if (getOrdForDoc(doc) == -1) {
    return L"";
  }
  shared_ptr<BytesRef> *const term = termsIndex->binaryValue();
  spareChars->copyUTF8Bytes(term);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return spareChars->toString();
}

bool DocTermsIndexDocValues::boolVal(int doc) 
{
  return exists(doc);
}

shared_ptr<ValueSourceScorer> DocTermsIndexDocValues::getRangeScorer(
    shared_ptr<LeafReaderContext> readerContext, const wstring &lowerVal,
    const wstring &upperVal, bool includeLower,
    bool includeUpper) 
{
  // TODO: are lowerVal and upperVal in indexed form or not?
  lowerVal = lowerVal == L"" ? L"" : toTerm(lowerVal);
  upperVal = upperVal == L"" ? L"" : toTerm(upperVal);

  int lower = numeric_limits<int>::min();
  if (lowerVal != L"") {
    lower = termsIndex->lookupTerm(make_shared<BytesRef>(lowerVal));
    if (lower < 0) {
      lower = -lower - 1;
    } else if (!includeLower) {
      lower++;
    }
  }

  int upper = numeric_limits<int>::max();
  if (upperVal != L"") {
    upper = termsIndex->lookupTerm(make_shared<BytesRef>(upperVal));
    if (upper < 0) {
      upper = -upper - 2;
    } else if (!includeUpper) {
      upper--;
    }
  }

  constexpr int ll = lower;
  constexpr int uu = upper;

  return make_shared<ValueSourceScorerAnonymousInnerClass>(
      shared_from_this(), readerContext, ll, uu);
}

DocTermsIndexDocValues::ValueSourceScorerAnonymousInnerClass::
    ValueSourceScorerAnonymousInnerClass(
        shared_ptr<DocTermsIndexDocValues> outerInstance,
        shared_ptr<LeafReaderContext> readerContext, int ll, int uu)
    : org::apache::lucene::queries::function::ValueSourceScorer(readerContext,
                                                                outerInstance)
{
  this->outerInstance = outerInstance;
  this->readerContext = readerContext;
  this->ll = ll;
  this->uu = uu;
  values = readerContext->reader()->getSortedDocValues(outerInstance->field);
}

bool DocTermsIndexDocValues::ValueSourceScorerAnonymousInnerClass::matches(
    int doc) 
{
  if (doc < outerInstance->lastDocID) {
    throw invalid_argument(L"docs were sent out-of-order: lastDocID=" +
                           to_wstring(outerInstance->lastDocID) +
                           L" vs docID=" + to_wstring(doc));
  }
  if (doc > values::docID()) {
    values::advance(doc);
  }
  if (doc == values::docID()) {
    int ord = values::ordValue();
    return ord >= ll && ord <= uu;
  } else {
    return false;
  }
}

wstring DocTermsIndexDocValues::toString(int doc) 
{
  return vs->description() + StringHelper::toString(L'=') + strVal(doc);
}

shared_ptr<FunctionValues::ValueFiller> DocTermsIndexDocValues::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

DocTermsIndexDocValues::ValueFillerAnonymousInnerClass::
    ValueFillerAnonymousInnerClass(
        shared_ptr<DocTermsIndexDocValues> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueStr>();
}

shared_ptr<MutableValue>
DocTermsIndexDocValues::ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void DocTermsIndexDocValues::ValueFillerAnonymousInnerClass::fillValue(
    int doc) 
{
  int ord = outerInstance->getOrdForDoc(doc);
  mval::value->clear();
  mval->exists = ord >= 0;
  if (mval::exists) {
    mval::value::copyBytes(outerInstance->termsIndex->lookupOrd(ord));
  }
}

shared_ptr<SortedDocValues>
DocTermsIndexDocValues::open(shared_ptr<LeafReaderContext> context,
                             const wstring &field) 
{
  try {
    return DocValues::getSorted(context->reader(), field);
  } catch (const runtime_error &e) {
    throw make_shared<DocTermsIndexException>(field, e);
  }
}

DocTermsIndexDocValues::DocTermsIndexException::DocTermsIndexException(
    const wstring &fieldName, runtime_error const cause)
    : RuntimeException(L"Can't initialize DocTermsIndex to generate (function) "
                       L"FunctionValues for field: " +
                           fieldName,
                       cause)
{
}
} // namespace org::apache::lucene::queries::function::docvalues