using namespace std;

#include "FunctionRangeQuery.h"

namespace org::apache::lucene::queries::function
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;

FunctionRangeQuery::FunctionRangeQuery(shared_ptr<ValueSource> valueSource,
                                       shared_ptr<Number> lowerVal,
                                       shared_ptr<Number> upperVal,
                                       bool includeLower, bool includeUpper)
    : FunctionRangeQuery(valueSource,
                         lowerVal == nullptr ? nullptr : lowerVal->toString(),
                         upperVal == nullptr ? nullptr : upperVal->toString(),
                         includeLower, includeUpper)
{
}

FunctionRangeQuery::FunctionRangeQuery(shared_ptr<ValueSource> valueSource,
                                       const wstring &lowerVal,
                                       const wstring &upperVal,
                                       bool includeLower, bool includeUpper)
    : valueSource(valueSource), lowerVal(lowerVal), upperVal(upperVal),
      includeLower(includeLower), includeUpper(includeUpper)
{
}

shared_ptr<ValueSource> FunctionRangeQuery::getValueSource()
{
  return valueSource;
}

wstring FunctionRangeQuery::getLowerVal() { return lowerVal; }

wstring FunctionRangeQuery::getUpperVal() { return upperVal; }

bool FunctionRangeQuery::isIncludeLower() { return includeLower; }

bool FunctionRangeQuery::isIncludeUpper() { return includeUpper; }

wstring FunctionRangeQuery::toString(const wstring &field)
{
  return L"frange(" + valueSource + L"):" + (includeLower ? L'[' : L'{') +
         (lowerVal == L"" ? L"*" : lowerVal) + L" TO " +
         (upperVal == L"" ? L"*" : upperVal) + (includeUpper ? L']' : L'}');
}

bool FunctionRangeQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool FunctionRangeQuery::equalsTo(shared_ptr<FunctionRangeQuery> other)
{
  return Objects::equals(includeLower, other->includeLower) &&
         Objects::equals(includeUpper, other->includeUpper) &&
         Objects::equals(valueSource, other->valueSource) &&
         Objects::equals(lowerVal, other->lowerVal) &&
         Objects::equals(upperVal, other->upperVal);
}

int FunctionRangeQuery::hashCode()
{
  return classHash() ^ Objects::hash(valueSource, lowerVal, upperVal,
                                     includeLower, includeUpper);
}

shared_ptr<Weight>
FunctionRangeQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                 bool needsScores,
                                 float boost) 
{
  return make_shared<FunctionRangeWeight>(shared_from_this(), searcher);
}

FunctionRangeQuery::FunctionRangeWeight::FunctionRangeWeight(
    shared_ptr<FunctionRangeQuery> outerInstance,
    shared_ptr<IndexSearcher> searcher) 
    : org::apache::lucene::search::Weight(FunctionRangeQuery::this),
      vsContext(ValueSource::newContext(searcher)), outerInstance(outerInstance)
{
  outerInstance->valueSource->createWeight(
      vsContext, searcher); // callback on valueSource tree
}

void FunctionRangeQuery::FunctionRangeWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  // none
}

shared_ptr<Explanation> FunctionRangeQuery::FunctionRangeWeight::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  shared_ptr<FunctionValues> functionValues =
      outerInstance->valueSource->getValues(vsContext, context);
  // note: by using ValueSourceScorer directly, we avoid calling
  // scorer.advance(doc) and checking if true,
  //  which can be slow since if that doc doesn't match, it has to linearly find
  //  the next matching
  shared_ptr<ValueSourceScorer> scorer = this->scorer(context);
  if (scorer->matches(doc)) {
    scorer->begin().advance(doc);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return Explanation::match(scorer->score(), outerInstance->toString(),
                              functionValues->explain(doc));
  } else {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return Explanation::noMatch(outerInstance->toString(),
                                functionValues->explain(doc));
  }
}

shared_ptr<ValueSourceScorer> FunctionRangeQuery::FunctionRangeWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<FunctionValues> functionValues =
      outerInstance->valueSource->getValues(vsContext, context);
  // getRangeScorer takes std::wstring args and parses them. Weird.
  return functionValues->getRangeScorer(
      context, outerInstance->lowerVal, outerInstance->upperVal,
      outerInstance->includeLower, outerInstance->includeUpper);
}

bool FunctionRangeQuery::FunctionRangeWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}
} // namespace org::apache::lucene::queries::function