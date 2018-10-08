using namespace std;

#include "FunctionScoreQuery.h"

namespace org::apache::lucene::queries::function
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using FilterScorer = org::apache::lucene::search::FilterScorer;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Matches = org::apache::lucene::search::Matches;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;

FunctionScoreQuery::FunctionScoreQuery(shared_ptr<Query> in_,
                                       shared_ptr<DoubleValuesSource> source)
    : in_(in_), source(source)
{
}

shared_ptr<Query> FunctionScoreQuery::getWrappedQuery() { return in_; }

shared_ptr<FunctionScoreQuery>
FunctionScoreQuery::boostByValue(shared_ptr<Query> in_,
                                 shared_ptr<DoubleValuesSource> boost)
{
  return make_shared<FunctionScoreQuery>(
      in_, make_shared<MultiplicativeBoostValuesSource>(boost));
}

shared_ptr<FunctionScoreQuery>
FunctionScoreQuery::boostByQuery(shared_ptr<Query> in_,
                                 shared_ptr<Query> boostMatch, float boostValue)
{
  return make_shared<FunctionScoreQuery>(
      in_, make_shared<MultiplicativeBoostValuesSource>(
               make_shared<QueryBoostValuesSource>(
                   DoubleValuesSource::fromQuery(boostMatch), boostValue)));
}

shared_ptr<Weight>
FunctionScoreQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                 bool needsScores,
                                 float boost) 
{
  shared_ptr<Weight> inner =
      in_->createWeight(searcher, needsScores && source->needsScores(), 1.0f);
  if (needsScores == false) {
    return inner;
  }
  return make_shared<FunctionScoreWeight>(shared_from_this(), inner,
                                          source->rewrite(searcher), boost);
}

shared_ptr<Query>
FunctionScoreQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> rewritten = in_->rewrite(reader);
  if (rewritten == in_) {
    return shared_from_this();
  }
  return make_shared<FunctionScoreQuery>(rewritten, source);
}

wstring FunctionScoreQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"FunctionScoreQuery(" + in_->toString(field) + L", scored by " +
         source->toString() + L")";
}

bool FunctionScoreQuery::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<FunctionScoreQuery> that =
      any_cast<std::shared_ptr<FunctionScoreQuery>>(o);
  return Objects::equals(in_, that->in_) &&
         Objects::equals(source, that->source);
}

int FunctionScoreQuery::hashCode() { return Objects::hash(in_, source); }

FunctionScoreQuery::FunctionScoreWeight::FunctionScoreWeight(
    shared_ptr<Query> query, shared_ptr<Weight> inner,
    shared_ptr<DoubleValuesSource> valueSource, float boost)
    : org::apache::lucene::search::Weight(query), inner(inner),
      valueSource(valueSource), boost(boost)
{
}

void FunctionScoreQuery::FunctionScoreWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  this->inner->extractTerms(terms);
}

shared_ptr<Matches> FunctionScoreQuery::FunctionScoreWeight::matches(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  return inner->matches(context, doc);
}

shared_ptr<Explanation> FunctionScoreQuery::FunctionScoreWeight::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  shared_ptr<Scorer> scorer = inner->scorer(context);
  if (scorer->begin().advance(doc) != doc) {
    return Explanation::noMatch(L"No match");
  }
  shared_ptr<Explanation> scoreExplanation = inner->explain(context, doc);
  shared_ptr<Explanation> expl =
      valueSource->explain(context, doc, scoreExplanation);
  return Explanation::match(expl->getValue() * boost, L"product of:",
                            {Explanation::match(boost, L"boost"), expl});
}

shared_ptr<Scorer> FunctionScoreQuery::FunctionScoreWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Scorer> in_ = inner->scorer(context);
  if (in_ == nullptr) {
    return nullptr;
  }
  shared_ptr<DoubleValues> scores =
      valueSource->getValues(context, DoubleValuesSource::fromScorer(in_));
  return make_shared<FilterScorerAnonymousInnerClass>(shared_from_this(), in_,
                                                      scores);
}

FunctionScoreQuery::FunctionScoreWeight::FilterScorerAnonymousInnerClass::
    FilterScorerAnonymousInnerClass(
        shared_ptr<FunctionScoreWeight> outerInstance, shared_ptr<Scorer> in_,
        shared_ptr<DoubleValues> scores)
    : org::apache::lucene::search::FilterScorer(in_)
{
  this->outerInstance = outerInstance;
  this->scores = scores;
}

float FunctionScoreQuery::FunctionScoreWeight::FilterScorerAnonymousInnerClass::
    score() 
{
  if (scores->advanceExact(docID())) {
    return static_cast<float>(scores->doubleValue() * outerInstance->boost);
  } else {
    return 0;
  }
}

bool FunctionScoreQuery::FunctionScoreWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return inner->isCacheable(ctx) && valueSource->isCacheable(ctx);
}

FunctionScoreQuery::MultiplicativeBoostValuesSource::
    MultiplicativeBoostValuesSource(shared_ptr<DoubleValuesSource> boost)
    : boost(boost)
{
}

shared_ptr<DoubleValues>
FunctionScoreQuery::MultiplicativeBoostValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  shared_ptr<DoubleValues> in_ =
      DoubleValues::withDefault(boost->getValues(ctx, scores), 1);
  return make_shared<DoubleValuesAnonymousInnerClass>(shared_from_this(),
                                                      scores, in_);
}

FunctionScoreQuery::MultiplicativeBoostValuesSource::
    DoubleValuesAnonymousInnerClass::DoubleValuesAnonymousInnerClass(
        shared_ptr<MultiplicativeBoostValuesSource> outerInstance,
        shared_ptr<DoubleValues> scores, shared_ptr<DoubleValues> in_)
{
  this->outerInstance = outerInstance;
  this->scores = scores;
  this->in_ = in_;
}

double FunctionScoreQuery::MultiplicativeBoostValuesSource::
    DoubleValuesAnonymousInnerClass::doubleValue() 
{
  return scores->doubleValue() * in_->doubleValue();
}

bool FunctionScoreQuery::MultiplicativeBoostValuesSource::
    DoubleValuesAnonymousInnerClass::advanceExact(int doc) 
{
  return in_->advanceExact(doc);
}

bool FunctionScoreQuery::MultiplicativeBoostValuesSource::needsScores()
{
  return true;
}

shared_ptr<DoubleValuesSource>
FunctionScoreQuery::MultiplicativeBoostValuesSource::rewrite(
    shared_ptr<IndexSearcher> reader) 
{
  return make_shared<MultiplicativeBoostValuesSource>(boost->rewrite(reader));
}

bool FunctionScoreQuery::MultiplicativeBoostValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<MultiplicativeBoostValuesSource> that =
      any_cast<std::shared_ptr<MultiplicativeBoostValuesSource>>(o);
  return Objects::equals(boost, that->boost);
}

shared_ptr<Explanation>
FunctionScoreQuery::MultiplicativeBoostValuesSource::explain(
    shared_ptr<LeafReaderContext> ctx, int docId,
    shared_ptr<Explanation> scoreExplanation) 
{
  if (scoreExplanation->isMatch() == false) {
    return scoreExplanation;
  }
  shared_ptr<Explanation> boostExpl =
      boost->explain(ctx, docId, scoreExplanation);
  if (boostExpl->isMatch() == false) {
    return scoreExplanation;
  }
  return Explanation::match(scoreExplanation->getValue() *
                                boostExpl->getValue(),
                            L"product of:", {scoreExplanation, boostExpl});
}

int FunctionScoreQuery::MultiplicativeBoostValuesSource::hashCode()
{
  return Objects::hash(boost);
}

wstring FunctionScoreQuery::MultiplicativeBoostValuesSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"boost(" + boost->toString() + L")";
}

bool FunctionScoreQuery::MultiplicativeBoostValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return boost->isCacheable(ctx);
}

FunctionScoreQuery::QueryBoostValuesSource::QueryBoostValuesSource(
    shared_ptr<DoubleValuesSource> query, float boost)
    : query(query), boost(boost)
{
}

shared_ptr<DoubleValues> FunctionScoreQuery::QueryBoostValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  shared_ptr<DoubleValues> in_ = query->getValues(ctx, nullptr);
  return DoubleValues::withDefault(
      make_shared<DoubleValuesAnonymousInnerClass>(shared_from_this(), in_), 1);
}

FunctionScoreQuery::QueryBoostValuesSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<QueryBoostValuesSource> outerInstance,
        shared_ptr<DoubleValues> in_)
{
  this->outerInstance = outerInstance;
  this->in_ = in_;
}

double FunctionScoreQuery::QueryBoostValuesSource::
    DoubleValuesAnonymousInnerClass::doubleValue()
{
  return outerInstance->boost;
}

bool FunctionScoreQuery::QueryBoostValuesSource::
    DoubleValuesAnonymousInnerClass::advanceExact(int doc) 
{
  return in_->advanceExact(doc);
}

bool FunctionScoreQuery::QueryBoostValuesSource::needsScores() { return false; }

shared_ptr<DoubleValuesSource>
FunctionScoreQuery::QueryBoostValuesSource::rewrite(
    shared_ptr<IndexSearcher> reader) 
{
  return make_shared<QueryBoostValuesSource>(query->rewrite(reader), boost);
}

bool FunctionScoreQuery::QueryBoostValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<QueryBoostValuesSource> that =
      any_cast<std::shared_ptr<QueryBoostValuesSource>>(o);
  return Float::compare(that->boost, boost) == 0 &&
         Objects::equals(query, that->query);
}

int FunctionScoreQuery::QueryBoostValuesSource::hashCode()
{
  return Objects::hash(query, boost);
}

wstring FunctionScoreQuery::QueryBoostValuesSource::toString()
{
  return L"queryboost(" + query + L")^" + to_wstring(boost);
}

bool FunctionScoreQuery::QueryBoostValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return query->isCacheable(ctx);
}

shared_ptr<Explanation> FunctionScoreQuery::QueryBoostValuesSource::explain(
    shared_ptr<LeafReaderContext> ctx, int docId,
    shared_ptr<Explanation> scoreExplanation) 
{
  shared_ptr<Explanation> inner = query->explain(ctx, docId, scoreExplanation);
  if (inner->isMatch() == false) {
    return inner;
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Explanation::match(boost,
                            L"Matched boosting query " + query->toString());
}
} // namespace org::apache::lucene::queries::function