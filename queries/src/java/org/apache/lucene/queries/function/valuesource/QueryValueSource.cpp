using namespace std;

#include "QueryValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueFloat =
    org::apache::lucene::util::mutable_::MutableValueFloat;

QueryValueSource::QueryValueSource(shared_ptr<Query> q, float defVal)
    : q(q), defVal(defVal)
{
}

shared_ptr<Query> QueryValueSource::getQuery() { return q; }

float QueryValueSource::getDefaultValue() { return defVal; }

wstring QueryValueSource::description()
{
  return L"query(" + q + L",def=" + to_wstring(defVal) + L")";
}

shared_ptr<FunctionValues> QueryValueSource::getValues(
    unordered_map fcontext,
    shared_ptr<LeafReaderContext> readerContext) 
{
  return make_shared<QueryDocValues>(shared_from_this(), readerContext,
                                     fcontext);
}

int QueryValueSource::hashCode() { return q->hashCode() * 29; }

bool QueryValueSource::equals(any o)
{
  if (QueryValueSource::typeid != o.type()) {
    return false;
  }
  shared_ptr<QueryValueSource> other =
      any_cast<std::shared_ptr<QueryValueSource>>(o);
  return this->q->equals(other->q) && this->defVal == other->defVal;
}

void QueryValueSource::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  shared_ptr<Query> rewritten = searcher->rewrite(q);
  shared_ptr<Weight> w = searcher->createWeight(rewritten, true, 1);
  context.emplace(shared_from_this(), w);
}

QueryDocValues::QueryDocValues(shared_ptr<QueryValueSource> vs,
                               shared_ptr<LeafReaderContext> readerContext,
                               unordered_map fcontext) 
    : org::apache::lucene::queries::function::docvalues::FloatDocValues(vs),
      readerContext(readerContext), weight(w), defVal(vs->defVal),
      fcontext(fcontext), q(vs->q)
{

  shared_ptr<Weight> w = fcontext.empty()
                             ? nullptr
                             : std::static_pointer_cast<Weight>(fcontext[vs]);
  if (w == nullptr) {
    shared_ptr<IndexSearcher> weightSearcher;
    if (fcontext.empty()) {
      weightSearcher = make_shared<IndexSearcher>(
          ReaderUtil::getTopLevelContext(readerContext));
    } else {
      weightSearcher =
          std::static_pointer_cast<IndexSearcher>(fcontext[L"searcher"]);
      if (weightSearcher == nullptr) {
        weightSearcher = make_shared<IndexSearcher>(
            ReaderUtil::getTopLevelContext(readerContext));
      }
    }
    vs->createWeight(fcontext, weightSearcher);
    w = std::static_pointer_cast<Weight>(fcontext[vs]);
  }
}

float QueryDocValues::floatVal(int doc)
{
  try {
    if (doc < lastDocRequested) {
      if (noMatches) {
        return defVal;
      }
      scorer = weight->scorer(readerContext);
      if (scorer == nullptr) {
        noMatches = true;
        return defVal;
      }
      it = scorer->begin();
      scorerDoc = -1;
    }
    lastDocRequested = doc;

    if (scorerDoc < doc) {
      scorerDoc = it->advance(doc);
    }

    if (scorerDoc > doc) {
      // query doesn't match this document... either because we hit the
      // end, or because the next doc is after this doc.
      return defVal;
    }

    // a match!
    return scorer->score();
  } catch (const IOException &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("caught exception in
    // QueryDocVals("+q+") doc="+doc, e);
    throw runtime_error(L"caught exception in QueryDocVals(" + q + L") doc=" +
                        to_wstring(doc));
  }
}

bool QueryDocValues::exists(int doc)
{
  try {
    if (doc < lastDocRequested) {
      if (noMatches) {
        return false;
      }
      scorer = weight->scorer(readerContext);
      scorerDoc = -1;
      if (scorer == nullptr) {
        noMatches = true;
        return false;
      }
      it = scorer->begin();
    }
    lastDocRequested = doc;

    if (scorerDoc < doc) {
      scorerDoc = it->advance(doc);
    }

    if (scorerDoc > doc) {
      // query doesn't match this document... either because we hit the
      // end, or because the next doc is after this doc.
      return false;
    }

    // a match!
    return true;
  } catch (const IOException &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("caught exception in
    // QueryDocVals("+q+") doc="+doc, e);
    throw runtime_error(L"caught exception in QueryDocVals(" + q + L") doc=" +
                        to_wstring(doc));
  }
}

any QueryDocValues::objectVal(int doc)
{
  try {
    return exists(doc) ? scorer->score() : nullptr;
  } catch (const IOException &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("caught exception in
    // QueryDocVals("+q+") doc="+doc, e);
    throw runtime_error(L"caught exception in QueryDocVals(" + q + L") doc=" +
                        to_wstring(doc));
  }
}

shared_ptr<FunctionValues::ValueFiller> QueryDocValues::getValueFiller()
{
  //
  // TODO: if we want to support more than one value-filler or a value-filler in
  // conjunction with the FunctionValues, then members like "scorer" should be
  // per ValueFiller instance. Or we can say that the user should just
  // instantiate multiple FunctionValues.
  //
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

QueryDocValues::ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
    shared_ptr<QueryDocValues> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueFloat>();
}

shared_ptr<MutableValue>
QueryDocValues::ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void QueryDocValues::ValueFillerAnonymousInnerClass::fillValue(int doc)
{
  try {
    if (outerInstance->noMatches) {
      mval->value = outerInstance->defVal;
      mval->exists = false;
      return;
    }
    outerInstance->scorer =
        outerInstance->weight->scorer(outerInstance->readerContext);
    outerInstance->scorerDoc = -1;
    if (outerInstance->scorer == nullptr) {
      outerInstance->noMatches = true;
      mval->value = outerInstance->defVal;
      mval->exists = false;
      return;
    }
    outerInstance->it = outerInstance->scorer->begin();
    outerInstance->lastDocRequested = doc;

    if (outerInstance->scorerDoc < doc) {
      outerInstance->scorerDoc = outerInstance->it->advance(doc);
    }

    if (outerInstance->scorerDoc > doc) {
      // query doesn't match this document... either because we hit the
      // end, or because the next doc is after this doc.
      mval->value = outerInstance->defVal;
      mval->exists = false;
      return;
    }

    // a match!
    mval->value = outerInstance->scorer->score();
    mval->exists = true;
  } catch (const IOException &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("caught exception in
    // QueryDocVals("+q+") doc="+doc, e);
    throw runtime_error(L"caught exception in QueryDocVals(" +
                        outerInstance->q + L") doc=" + to_wstring(doc));
  }
}

wstring QueryDocValues::toString(int doc)
{
  return L"query(" + q + L",def=" + to_wstring(defVal) + L")=" +
         to_wstring(floatVal(doc));
}
} // namespace org::apache::lucene::queries::function::valuesource