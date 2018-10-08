using namespace std;

#include "IndexReaderFunctions.h"

namespace org::apache::lucene::queries::function
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using LongValues = org::apache::lucene::search::LongValues;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;

IndexReaderFunctions::IndexReaderFunctions() {}

shared_ptr<DoubleValuesSource>
IndexReaderFunctions::docFreq(shared_ptr<Term> term)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return make_shared<IndexReaderDoubleValuesSource>(
      [&](any r) { static_cast<double>(r::docFreq(term)); },
      L"docFreq(" + term->toString() + L")");
}

shared_ptr<DoubleValuesSource> IndexReaderFunctions::maxDoc()
{
  return make_shared<IndexReaderDoubleValuesSource>(IndexReader::maxDoc,
                                                    L"maxDoc()");
}

shared_ptr<DoubleValuesSource> IndexReaderFunctions::numDocs()
{
  return make_shared<IndexReaderDoubleValuesSource>(IndexReader::numDocs,
                                                    L"numDocs()");
}

shared_ptr<DoubleValuesSource> IndexReaderFunctions::numDeletedDocs()
{
  return make_shared<IndexReaderDoubleValuesSource>(IndexReader::numDeletedDocs,
                                                    L"numDeletedDocs()");
}

shared_ptr<LongValuesSource>
IndexReaderFunctions::sumTotalTermFreq(const wstring &field)
{
  return make_shared<SumTotalTermFreqValuesSource>(field);
}

IndexReaderFunctions::SumTotalTermFreqValuesSource::
    SumTotalTermFreqValuesSource(const wstring &field)
    : field(field)
{
}

shared_ptr<LongValues>
IndexReaderFunctions::SumTotalTermFreqValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  throw make_shared<UnsupportedOperationException>(
      L"IndexReaderFunction must be rewritten before use");
}

bool IndexReaderFunctions::SumTotalTermFreqValuesSource::needsScores()
{
  return false;
}

bool IndexReaderFunctions::SumTotalTermFreqValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<SumTotalTermFreqValuesSource> that =
      any_cast<std::shared_ptr<SumTotalTermFreqValuesSource>>(o);
  return Objects::equals(field, that->field);
}

int IndexReaderFunctions::SumTotalTermFreqValuesSource::hashCode()
{
  return Objects::hash(field);
}

shared_ptr<LongValuesSource>
IndexReaderFunctions::SumTotalTermFreqValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return make_shared<NoCacheConstantLongValuesSource>(
      searcher->getIndexReader()->getSumTotalTermFreq(field),
      shared_from_this());
}

wstring IndexReaderFunctions::SumTotalTermFreqValuesSource::toString()
{
  return L"sumTotalTermFreq(" + field + L")";
}

bool IndexReaderFunctions::SumTotalTermFreqValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

IndexReaderFunctions::NoCacheConstantLongValuesSource::
    NoCacheConstantLongValuesSource(int64_t value,
                                    shared_ptr<LongValuesSource> parent)
    : value(value), parent(parent)
{
}

shared_ptr<LongValues>
IndexReaderFunctions::NoCacheConstantLongValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  return make_shared<LongValuesAnonymousInnerClass>(shared_from_this());
}

IndexReaderFunctions::NoCacheConstantLongValuesSource::
    LongValuesAnonymousInnerClass::LongValuesAnonymousInnerClass(
        shared_ptr<NoCacheConstantLongValuesSource> outerInstance)
{
  this->outerInstance = outerInstance;
}

int64_t IndexReaderFunctions::NoCacheConstantLongValuesSource::
    LongValuesAnonymousInnerClass::longValue() 
{
  return outerInstance->value;
}

bool IndexReaderFunctions::NoCacheConstantLongValuesSource::
    LongValuesAnonymousInnerClass::advanceExact(int doc) 
{
  return true;
}

bool IndexReaderFunctions::NoCacheConstantLongValuesSource::needsScores()
{
  return false;
}

shared_ptr<LongValuesSource>
IndexReaderFunctions::NoCacheConstantLongValuesSource::rewrite(
    shared_ptr<IndexSearcher> reader) 
{
  return shared_from_this();
}

bool IndexReaderFunctions::NoCacheConstantLongValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (!(std::dynamic_pointer_cast<NoCacheConstantLongValuesSource>(o) !=
        nullptr)) {
    return false;
  }
  shared_ptr<NoCacheConstantLongValuesSource> that =
      any_cast<std::shared_ptr<NoCacheConstantLongValuesSource>>(o);
  return value == that->value && Objects::equals(parent, that->parent);
}

int IndexReaderFunctions::NoCacheConstantLongValuesSource::hashCode()
{
  return Objects::hash(value, parent);
}

wstring IndexReaderFunctions::NoCacheConstantLongValuesSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return parent->toString();
}

bool IndexReaderFunctions::NoCacheConstantLongValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

shared_ptr<DoubleValuesSource>
IndexReaderFunctions::termFreq(shared_ptr<Term> term)
{
  return make_shared<TermFreqDoubleValuesSource>(term);
}

IndexReaderFunctions::TermFreqDoubleValuesSource::TermFreqDoubleValuesSource(
    shared_ptr<Term> term)
    : term(term)
{
}

shared_ptr<DoubleValues>
IndexReaderFunctions::TermFreqDoubleValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  shared_ptr<Terms> terms = ctx->reader()->terms(term->field());
  shared_ptr<TermsEnum> te = terms == nullptr ? nullptr : terms->begin();

  if (te == nullptr || te->seekExact(term->bytes()) == false) {
    return DoubleValues::EMPTY;
  }

  shared_ptr<PostingsEnum> *const pe = te->postings(nullptr);
  assert(pe != nullptr);

  return make_shared<DoubleValuesAnonymousInnerClass>(shared_from_this(), pe);
}

IndexReaderFunctions::TermFreqDoubleValuesSource::
    DoubleValuesAnonymousInnerClass::DoubleValuesAnonymousInnerClass(
        shared_ptr<TermFreqDoubleValuesSource> outerInstance,
        shared_ptr<PostingsEnum> pe)
{
  this->outerInstance = outerInstance;
  this->pe = pe;
}

double IndexReaderFunctions::TermFreqDoubleValuesSource::
    DoubleValuesAnonymousInnerClass::doubleValue() 
{
  return pe->freq();
}

bool IndexReaderFunctions::TermFreqDoubleValuesSource::
    DoubleValuesAnonymousInnerClass::advanceExact(int doc) 
{
  if (pe->docID() > doc) {
    return false;
  }
  return pe->docID() == doc || pe->advance(doc) == doc;
}

bool IndexReaderFunctions::TermFreqDoubleValuesSource::needsScores()
{
  return false;
}

shared_ptr<DoubleValuesSource>
IndexReaderFunctions::TermFreqDoubleValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

wstring IndexReaderFunctions::TermFreqDoubleValuesSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"termFreq(" + term->toString() + L")";
}

bool IndexReaderFunctions::TermFreqDoubleValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<TermFreqDoubleValuesSource> that =
      any_cast<std::shared_ptr<TermFreqDoubleValuesSource>>(o);
  return Objects::equals(term, that->term);
}

int IndexReaderFunctions::TermFreqDoubleValuesSource::hashCode()
{
  return Objects::hash(term);
}

bool IndexReaderFunctions::TermFreqDoubleValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

shared_ptr<DoubleValuesSource>
IndexReaderFunctions::totalTermFreq(shared_ptr<Term> term)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return make_shared<IndexReaderDoubleValuesSource>(
      [&](any r) { r::totalTermFreq(term); },
      L"totalTermFreq(" + term->toString() + L")");
}

shared_ptr<DoubleValuesSource>
IndexReaderFunctions::sumDocFreq(const wstring &field)
{
  return make_shared<IndexReaderDoubleValuesSource>(
      [&](any r) { r::getSumDocFreq(field); }, L"sumDocFreq(" + field + L")");
}

shared_ptr<DoubleValuesSource>
IndexReaderFunctions::docCount(const wstring &field)
{
  return make_shared<IndexReaderDoubleValuesSource>(
      [&](any r) { r::getDocCount(field); }, L"docCount(" + field + L")");
}

IndexReaderFunctions::IndexReaderDoubleValuesSource::
    IndexReaderDoubleValuesSource(ReaderFunction func,
                                  const wstring &description)
    : func(func), description(description)
{
}

shared_ptr<DoubleValues>
IndexReaderFunctions::IndexReaderDoubleValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  throw make_shared<UnsupportedOperationException>(
      L"IndexReaderFunction must be rewritten before use");
}

bool IndexReaderFunctions::IndexReaderDoubleValuesSource::needsScores()
{
  return false;
}

shared_ptr<DoubleValuesSource>
IndexReaderFunctions::IndexReaderDoubleValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return make_shared<NoCacheConstantDoubleValuesSource>(
      func(searcher->getIndexReader()), shared_from_this());
}

wstring IndexReaderFunctions::IndexReaderDoubleValuesSource::toString()
{
  return description;
}

bool IndexReaderFunctions::IndexReaderDoubleValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<IndexReaderDoubleValuesSource> that =
      any_cast<std::shared_ptr<IndexReaderDoubleValuesSource>>(o);
  return Objects::equals(description, that->description) &&
         Objects::equals(func, that->func);
}

int IndexReaderFunctions::IndexReaderDoubleValuesSource::hashCode()
{
  return Objects::hash(description, func);
}

bool IndexReaderFunctions::IndexReaderDoubleValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

IndexReaderFunctions::NoCacheConstantDoubleValuesSource::
    NoCacheConstantDoubleValuesSource(double value,
                                      shared_ptr<DoubleValuesSource> parent)
    : value(value), parent(parent)
{
}

shared_ptr<DoubleValues>
IndexReaderFunctions::NoCacheConstantDoubleValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  return make_shared<DoubleValuesAnonymousInnerClass>(shared_from_this());
}

IndexReaderFunctions::NoCacheConstantDoubleValuesSource::
    DoubleValuesAnonymousInnerClass::DoubleValuesAnonymousInnerClass(
        shared_ptr<NoCacheConstantDoubleValuesSource> outerInstance)
{
  this->outerInstance = outerInstance;
}

double IndexReaderFunctions::NoCacheConstantDoubleValuesSource::
    DoubleValuesAnonymousInnerClass::doubleValue() 
{
  return outerInstance->value;
}

bool IndexReaderFunctions::NoCacheConstantDoubleValuesSource::
    DoubleValuesAnonymousInnerClass::advanceExact(int doc) 
{
  return true;
}

bool IndexReaderFunctions::NoCacheConstantDoubleValuesSource::needsScores()
{
  return false;
}

shared_ptr<DoubleValuesSource>
IndexReaderFunctions::NoCacheConstantDoubleValuesSource::rewrite(
    shared_ptr<IndexSearcher> reader) 
{
  return shared_from_this();
}

bool IndexReaderFunctions::NoCacheConstantDoubleValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (!(std::dynamic_pointer_cast<NoCacheConstantDoubleValuesSource>(o) !=
        nullptr)) {
    return false;
  }
  shared_ptr<NoCacheConstantDoubleValuesSource> that =
      any_cast<std::shared_ptr<NoCacheConstantDoubleValuesSource>>(o);
  return Double::compare(that->value, value) == 0 &&
         Objects::equals(parent, that->parent);
}

int IndexReaderFunctions::NoCacheConstantDoubleValuesSource::hashCode()
{
  return Objects::hash(value, parent);
}

wstring IndexReaderFunctions::NoCacheConstantDoubleValuesSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return parent->toString();
}

bool IndexReaderFunctions::NoCacheConstantDoubleValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}
} // namespace org::apache::lucene::queries::function