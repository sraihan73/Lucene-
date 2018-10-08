using namespace std;

#include "TermFreqValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using IntDocValues =
    org::apache::lucene::queries::function::docvalues::IntDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using BytesRef = org::apache::lucene::util::BytesRef;

TermFreqValueSource::TermFreqValueSource(const wstring &field,
                                         const wstring &val,
                                         const wstring &indexedField,
                                         shared_ptr<BytesRef> indexedBytes)
    : DocFreqValueSource(field, val, indexedField, indexedBytes)
{
}

wstring TermFreqValueSource::name() { return L"termfreq"; }

shared_ptr<FunctionValues> TermFreqValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<Terms> *const terms = readerContext->reader()->terms(indexedField);

  return make_shared<IntDocValuesAnonymousInnerClass>(shared_from_this(),
                                                      terms);
}

TermFreqValueSource::IntDocValuesAnonymousInnerClass::
    IntDocValuesAnonymousInnerClass(
        shared_ptr<TermFreqValueSource> outerInstance, shared_ptr<Terms> terms)
    : org::apache::lucene::queries::function::docvalues::IntDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->terms = terms;
  lastDocRequested = -1;

  reset();
}

void TermFreqValueSource::IntDocValuesAnonymousInnerClass::reset() throw(
    IOException)
{
  // no one should call us for deleted docs?

  if (terms != nullptr) {
    shared_ptr<TermsEnum> *const termsEnum = terms->begin();
    if (termsEnum->seekExact(outerInstance->indexedBytes)) {
      docs = termsEnum->postings(nullptr);
    } else {
      docs = nullptr;
    }
  } else {
    docs = nullptr;
  }

  if (docs == nullptr) {
    docs = make_shared<PostingsEnumAnonymousInnerClass>(shared_from_this());
  }
  atDoc = -1;
}

TermFreqValueSource::IntDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::PostingsEnumAnonymousInnerClass(
        shared_ptr<IntDocValuesAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TermFreqValueSource::IntDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::freq()
{
  return 0;
}

int TermFreqValueSource::IntDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::nextPosition() 
{
  return -1;
}

int TermFreqValueSource::IntDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::startOffset() 
{
  return -1;
}

int TermFreqValueSource::IntDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::endOffset() 
{
  return -1;
}

shared_ptr<BytesRef> TermFreqValueSource::IntDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::getPayload() 
{
  throw make_shared<UnsupportedOperationException>();
}

int TermFreqValueSource::IntDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::docID()
{
  return DocIdSetIterator::NO_MORE_DOCS;
}

int TermFreqValueSource::IntDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::nextDoc()
{
  return DocIdSetIterator::NO_MORE_DOCS;
}

int TermFreqValueSource::IntDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::advance(int target)
{
  return DocIdSetIterator::NO_MORE_DOCS;
}

int64_t TermFreqValueSource::IntDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::cost()
{
  return 0;
}

int TermFreqValueSource::IntDocValuesAnonymousInnerClass::intVal(int doc)
{
  try {
    if (doc < lastDocRequested) {
      // out-of-order access.... reset
      reset();
    }
    lastDocRequested = doc;

    if (atDoc < doc) {
      atDoc = docs::advance(doc);
    }

    if (atDoc > doc) {
      // term doesn't match this document... either because we hit the
      // end, or because the next doc is after this doc.
      return 0;
    }

    // a match!
    return docs::freq();
  } catch (const IOException &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("caught exception in function
    // "+description()+" : doc="+doc, e);
    throw runtime_error(L"caught exception in function " +
                        outerInstance->description() + L" : doc=" +
                        to_wstring(doc));
  }
}
} // namespace org::apache::lucene::queries::function::valuesource