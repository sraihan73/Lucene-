using namespace std;

#include "TFValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TFIDFSimilarity =
    org::apache::lucene::search::similarities::TFIDFSimilarity;
using BytesRef = org::apache::lucene::util::BytesRef;

TFValueSource::TFValueSource(const wstring &field, const wstring &val,
                             const wstring &indexedField,
                             shared_ptr<BytesRef> indexedBytes)
    : TermFreqValueSource(field, val, indexedField, indexedBytes)
{
}

wstring TFValueSource::name() { return L"tf"; }

shared_ptr<FunctionValues> TFValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<Terms> *const terms = readerContext->reader()->terms(indexedField);
  shared_ptr<IndexSearcher> searcher =
      std::static_pointer_cast<IndexSearcher>(context[L"searcher"]);
  shared_ptr<TFIDFSimilarity> *const similarity =
      IDFValueSource::asTFIDF(searcher->getSimilarity(true), indexedField);
  if (similarity == nullptr) {
    throw make_shared<UnsupportedOperationException>(
        L"requires a TFIDFSimilarity (such as ClassicSimilarity)");
  }

  return make_shared<FloatDocValuesAnonymousInnerClass>(shared_from_this(),
                                                        terms, similarity);
}

TFValueSource::FloatDocValuesAnonymousInnerClass::
    FloatDocValuesAnonymousInnerClass(shared_ptr<TFValueSource> outerInstance,
                                      shared_ptr<Terms> terms,
                                      shared_ptr<TFIDFSimilarity> similarity)
    : org::apache::lucene::queries::function::docvalues::FloatDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->terms = terms;
  this->similarity = similarity;
  lastDocRequested = -1;

  reset();
}

void TFValueSource::FloatDocValuesAnonymousInnerClass::reset() throw(
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

TFValueSource::FloatDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::PostingsEnumAnonymousInnerClass(
        shared_ptr<FloatDocValuesAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TFValueSource::FloatDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::freq()
{
  return 0;
}

int TFValueSource::FloatDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::nextPosition() 
{
  return -1;
}

int TFValueSource::FloatDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::startOffset() 
{
  return -1;
}

int TFValueSource::FloatDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::endOffset() 
{
  return -1;
}

shared_ptr<BytesRef> TFValueSource::FloatDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::getPayload() 
{
  return nullptr;
}

int TFValueSource::FloatDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::docID()
{
  return DocIdSetIterator::NO_MORE_DOCS;
}

int TFValueSource::FloatDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::nextDoc()
{
  return DocIdSetIterator::NO_MORE_DOCS;
}

int TFValueSource::FloatDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::advance(int target)
{
  return DocIdSetIterator::NO_MORE_DOCS;
}

int64_t TFValueSource::FloatDocValuesAnonymousInnerClass::
    PostingsEnumAnonymousInnerClass::cost()
{
  return 0;
}

float TFValueSource::FloatDocValuesAnonymousInnerClass::floatVal(int doc)
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
      return similarity->tf(0);
    }

    // a match!
    return similarity->tf(docs::freq());
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