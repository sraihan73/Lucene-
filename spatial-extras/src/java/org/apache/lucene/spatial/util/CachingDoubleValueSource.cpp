using namespace std;

#include "CachingDoubleValueSource.h"

namespace org::apache::lucene::spatial::util
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

CachingDoubleValueSource::CachingDoubleValueSource(
    shared_ptr<DoubleValuesSource> source)
    : source(source), cache(unordered_map<>())
{
}

wstring CachingDoubleValueSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"Cached[" + source->toString() + L"]";
}

shared_ptr<DoubleValues> CachingDoubleValueSource::getValues(
    shared_ptr<LeafReaderContext> readerContext,
    shared_ptr<DoubleValues> scores) 
{
  constexpr int base = readerContext->docBase;
  shared_ptr<DoubleValues> *const vals =
      source->getValues(readerContext, scores);
  return make_shared<DoubleValuesAnonymousInnerClass>(shared_from_this(), base,
                                                      vals);
}

CachingDoubleValueSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<CachingDoubleValueSource> outerInstance, int base,
        shared_ptr<DoubleValues> vals)
{
  this->outerInstance = outerInstance;
  this->base = base;
  this->vals = vals;
  doc = -1;
}

double
CachingDoubleValueSource::DoubleValuesAnonymousInnerClass::doubleValue() throw(
    IOException)
{
  int key = base + doc;
  optional<double> v = outerInstance->cache[key];
  if (!v) {
    v = vals->doubleValue();
    outerInstance->cache.emplace(key, v);
  }
  return v;
}

bool CachingDoubleValueSource::DoubleValuesAnonymousInnerClass::advanceExact(
    int doc) 
{
  this->doc = doc;
  return vals->advanceExact(doc);
}

bool CachingDoubleValueSource::needsScores() { return false; }

bool CachingDoubleValueSource::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return source->isCacheable(ctx);
}

shared_ptr<Explanation> CachingDoubleValueSource::explain(
    shared_ptr<LeafReaderContext> ctx, int docId,
    shared_ptr<Explanation> scoreExplanation) 
{
  return source->explain(ctx, docId, scoreExplanation);
}

shared_ptr<DoubleValuesSource> CachingDoubleValueSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return make_shared<CachingDoubleValueSource>(source->rewrite(searcher));
}

bool CachingDoubleValueSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }

  shared_ptr<CachingDoubleValueSource> that =
      any_cast<std::shared_ptr<CachingDoubleValueSource>>(o);

  if (source != nullptr ? !source->equals(that->source)
                        : that->source != nullptr) {
    return false;
  }

  return true;
}

int CachingDoubleValueSource::hashCode()
{
  return source != nullptr ? source->hashCode() : 0;
}
} // namespace org::apache::lucene::spatial::util