using namespace std;

#include "FeatureQuery.h"

namespace org::apache::lucene::document
{
using FeatureFunction =
    org::apache::lucene::document::FeatureField::FeatureFunction;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;
using BytesRef = org::apache::lucene::util::BytesRef;

FeatureQuery::FeatureQuery(const wstring &fieldName, const wstring &featureName,
                           shared_ptr<FeatureFunction> function)
    : fieldName(Objects::requireNonNull(fieldName)),
      featureName(Objects::requireNonNull(featureName)),
      function(Objects::requireNonNull(function))
{
}

shared_ptr<Query>
FeatureQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<FeatureFunction> rewritten = function->rewrite(reader);
  if (function != rewritten) {
    return make_shared<FeatureQuery>(fieldName, featureName, rewritten);
  }
  return Query::rewrite(reader);
}

bool FeatureQuery::equals(any obj)
{
  if (obj == nullptr || getClass() != obj.type()) {
    return false;
  }
  shared_ptr<FeatureQuery> that = any_cast<std::shared_ptr<FeatureQuery>>(obj);
  return Objects::equals(fieldName, that->fieldName) &&
         Objects::equals(featureName, that->featureName) &&
         Objects::equals(function, that->function);
}

int FeatureQuery::hashCode()
{
  int h = getClass().hashCode();
  h = 31 * h + fieldName.hashCode();
  h = 31 * h + featureName.hashCode();
  h = 31 * h + function->hashCode();
  return h;
}

shared_ptr<Weight>
FeatureQuery::createWeight(shared_ptr<IndexSearcher> searcher, bool needsScores,
                           float boost) 
{
  return make_shared<WeightAnonymousInnerClass>(shared_from_this(), needsScores,
                                                boost);
}

FeatureQuery::WeightAnonymousInnerClass::WeightAnonymousInnerClass(
    shared_ptr<FeatureQuery> outerInstance, bool needsScores, float boost)
    : org::apache::lucene::search::Weight(outerInstance)
{
  this->outerInstance = outerInstance;
  this->needsScores = needsScores;
  this->boost = boost;
}

bool FeatureQuery::WeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

void FeatureQuery::WeightAnonymousInnerClass::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  if (needsScores == false) {
    // features are irrelevant to highlighting, skip
  } else {
    // extracting the term here will help get better scoring with
    // distributed term statistics if the saturation function is used
    // and the pivot value is computed automatically
    terms->add(make_shared<Term>(outerInstance->fieldName,
                                 outerInstance->featureName));
  }
}

shared_ptr<Explanation> FeatureQuery::WeightAnonymousInnerClass::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  wstring desc = L"weight(" + getQuery() + L" in " + to_wstring(doc) + L") [" +
                 outerInstance->function + L"]";

  shared_ptr<Terms> terms = context->reader()->terms(outerInstance->fieldName);
  if (terms == nullptr) {
    return Explanation::noMatch(desc + L". Field " + outerInstance->fieldName +
                                L" doesn't exist.");
  }
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  if (termsEnum->seekExact(make_shared<BytesRef>(outerInstance->featureName)) ==
      false) {
    return Explanation::noMatch(
        desc + L". Feature " + outerInstance->featureName + L" doesn't exist.");
  }

  shared_ptr<PostingsEnum> postings =
      termsEnum->postings(nullptr, PostingsEnum::FREQS);
  if (postings->advance(doc) != doc) {
    return Explanation::noMatch(desc + L". Feature " +
                                outerInstance->featureName + L" isn't set.");
  }

  return outerInstance->function->explain(outerInstance->fieldName,
                                          outerInstance->featureName, boost,
                                          doc, postings->freq());
}

shared_ptr<Scorer> FeatureQuery::WeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Terms> terms = context->reader()->terms(outerInstance->fieldName);
  if (terms == nullptr) {
    return nullptr;
  }
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  if (termsEnum->seekExact(make_shared<BytesRef>(outerInstance->featureName)) ==
      false) {
    return nullptr;
  }

  shared_ptr<SimScorer> scorer =
      outerInstance->function->scorer(outerInstance->fieldName, boost);
  shared_ptr<PostingsEnum> postings =
      termsEnum->postings(nullptr, PostingsEnum::FREQS);

  return make_shared<ScorerAnonymousInnerClass>(shared_from_this(), scorer,
                                                postings);
}

FeatureQuery::WeightAnonymousInnerClass::ScorerAnonymousInnerClass::
    ScorerAnonymousInnerClass(
        shared_ptr<WeightAnonymousInnerClass> outerInstance,
        shared_ptr<SimScorer> scorer, shared_ptr<PostingsEnum> postings)
    : org::apache::lucene::search::Scorer(outerInstance)
{
  this->outerInstance = outerInstance;
  this->scorer = scorer;
  this->postings = postings;
}

int FeatureQuery::WeightAnonymousInnerClass::ScorerAnonymousInnerClass::docID()
{
  return postings->docID();
}

float FeatureQuery::WeightAnonymousInnerClass::ScorerAnonymousInnerClass::
    score() 
{
  return scorer->score(postings->docID(), postings->freq());
}

shared_ptr<DocIdSetIterator>
FeatureQuery::WeightAnonymousInnerClass::ScorerAnonymousInnerClass::iterator()
{
  return postings;
}

wstring FeatureQuery::toString(const wstring &field)
{
  return L"FeatureQuery(field=" + fieldName + L", feature=" + featureName +
         L", function=" + function + L")";
}
} // namespace org::apache::lucene::document