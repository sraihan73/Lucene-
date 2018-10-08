using namespace std;

#include "TermsIncludingScoreQuery.h"

namespace org::apache::lucene::search::join
{
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
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

TermsIncludingScoreQuery::TermsIncludingScoreQuery(
    ScoreMode scoreMode, const wstring &toField, bool multipleValuesPerDocument,
    shared_ptr<BytesRefHash> terms, std::deque<float> &scores,
    const wstring &fromField, shared_ptr<Query> fromQuery,
    any indexReaderContextId)
    : scoreMode(scoreMode), toField(toField),
      multipleValuesPerDocument(multipleValuesPerDocument), terms(terms),
      scores(scores), ords(terms->sort()), fromQuery(fromQuery),
      fromField(fromField), topReaderContextId(indexReaderContextId)
{
}

wstring TermsIncludingScoreQuery::toString(const wstring &string)
{
  return wstring::format(Locale::ROOT,
                         L"TermsIncludingScoreQuery{field=%s;fromQuery=%s}",
                         toField, fromQuery);
}

bool TermsIncludingScoreQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool TermsIncludingScoreQuery::equalsTo(
    shared_ptr<TermsIncludingScoreQuery> other)
{
  return Objects::equals(scoreMode, other->scoreMode) &&
         Objects::equals(toField, other->toField) &&
         Objects::equals(fromField, other->fromField) &&
         Objects::equals(fromQuery, other->fromQuery) &&
         Objects::equals(topReaderContextId, other->topReaderContextId);
}

int TermsIncludingScoreQuery::hashCode()
{
  return classHash() + Objects::hash(scoreMode, toField, fromField, fromQuery,
                                     topReaderContextId);
}

shared_ptr<Weight>
TermsIncludingScoreQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost) 
{
  if (needsScores == false) {
    // We don't need scores then quickly change the query:
    shared_ptr<TermsQuery> termsQuery = make_shared<TermsQuery>(
        toField, terms, fromField, fromQuery, topReaderContextId);
    return searcher->rewrite(termsQuery)->createWeight(searcher, false, boost);
  }
  return make_shared<WeightAnonymousInnerClass>(shared_from_this(),
                                                TermsIncludingScoreQuery::this);
}

// C++ TODO: You cannot use 'shared_from_this' in a constructor:
TermsIncludingScoreQuery::WeightAnonymousInnerClass::WeightAnonymousInnerClass(
    shared_ptr<TermsIncludingScoreQuery> outerInstance,
    shared_ptr<org::apache::lucene::search::join::TermsIncludingScoreQuery>
        shared_from_this())
    : org::apache::lucene::search::Weight(this)
{
  this->outerInstance = outerInstance;
}

void TermsIncludingScoreQuery::WeightAnonymousInnerClass::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
}

shared_ptr<Explanation>
TermsIncludingScoreQuery::WeightAnonymousInnerClass::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  shared_ptr<Terms> terms = context->reader()->terms(outerInstance->toField);
  if (terms != nullptr) {
    shared_ptr<TermsEnum> segmentTermsEnum = terms->begin();
    shared_ptr<BytesRef> spare = make_shared<BytesRef>();
    shared_ptr<PostingsEnum> postingsEnum = nullptr;
    for (int i = 0; i < outerInstance->terms->size(); i++) {
      if (segmentTermsEnum->seekExact(
              outerInstance->terms->get(outerInstance->ords[i], spare))) {
        postingsEnum =
            segmentTermsEnum->postings(postingsEnum, PostingsEnum::NONE);
        if (postingsEnum->advance(doc) == doc) {
          constexpr float score = outerInstance->scores[outerInstance->ords[i]];
          return Explanation::match(
              score, L"Score based on join value " +
                         segmentTermsEnum->term()->utf8ToString());
        }
      }
    }
  }
  return Explanation::noMatch(L"Not a match");
}

shared_ptr<Scorer> TermsIncludingScoreQuery::WeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Terms> terms = context->reader()->terms(outerInstance->toField);
  if (terms == nullptr) {
    return nullptr;
  }

  // what is the runtime...seems ok?
  constexpr int64_t cost = context->reader()->maxDoc() * terms->size();

  shared_ptr<TermsEnum> segmentTermsEnum = terms->begin();
  if (outerInstance->multipleValuesPerDocument) {
    return make_shared<MVInOrderScorer>(outerInstance, shared_from_this(),
                                        segmentTermsEnum,
                                        context->reader()->maxDoc(), cost);
  } else {
    return make_shared<SVInOrderScorer>(outerInstance, shared_from_this(),
                                        segmentTermsEnum,
                                        context->reader()->maxDoc(), cost);
  }
}

bool TermsIncludingScoreQuery::WeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

TermsIncludingScoreQuery::SVInOrderScorer::SVInOrderScorer(
    shared_ptr<TermsIncludingScoreQuery> outerInstance,
    shared_ptr<Weight> weight, shared_ptr<TermsEnum> termsEnum, int maxDoc,
    int64_t cost) 
    : org::apache::lucene::search::Scorer(weight),
      matchingDocsIterator(make_shared<BitSetIterator>(matchingDocs, cost)),
      scores(std::deque<float>(maxDoc)), cost(cost),
      outerInstance(outerInstance)
{
  shared_ptr<FixedBitSet> matchingDocs = make_shared<FixedBitSet>(maxDoc);
  fillDocsAndScores(matchingDocs, termsEnum);
}

void TermsIncludingScoreQuery::SVInOrderScorer::fillDocsAndScores(
    shared_ptr<FixedBitSet> matchingDocs,
    shared_ptr<TermsEnum> termsEnum) 
{
  shared_ptr<BytesRef> spare = make_shared<BytesRef>();
  shared_ptr<PostingsEnum> postingsEnum = nullptr;
  for (int i = 0; i < outerInstance->terms->size(); i++) {
    if (termsEnum->seekExact(
            outerInstance->terms->get(outerInstance->ords[i], spare))) {
      postingsEnum = termsEnum->postings(postingsEnum, PostingsEnum::NONE);
      float score = outerInstance->scores[outerInstance->ords[i]];
      for (int doc = postingsEnum->nextDoc();
           doc != DocIdSetIterator::NO_MORE_DOCS;
           doc = postingsEnum->nextDoc()) {
        matchingDocs->set(doc);
        // In the case the same doc is also related to a another doc, a score
        // might be overwritten. I think this can only happen in a many-to-many
        // relation
        scores[doc] = score;
      }
    }
  }
}

float TermsIncludingScoreQuery::SVInOrderScorer::score() 
{
  return scores[docID()];
}

int TermsIncludingScoreQuery::SVInOrderScorer::docID()
{
  return matchingDocsIterator->docID();
}

shared_ptr<DocIdSetIterator>
TermsIncludingScoreQuery::SVInOrderScorer::iterator()
{
  return matchingDocsIterator;
}

TermsIncludingScoreQuery::MVInOrderScorer::MVInOrderScorer(
    shared_ptr<TermsIncludingScoreQuery> outerInstance,
    shared_ptr<Weight> weight, shared_ptr<TermsEnum> termsEnum, int maxDoc,
    int64_t cost) 
    : SVInOrderScorer(outerInstance, weight, termsEnum, maxDoc, cost),
      outerInstance(outerInstance)
{
}

void TermsIncludingScoreQuery::MVInOrderScorer::fillDocsAndScores(
    shared_ptr<FixedBitSet> matchingDocs,
    shared_ptr<TermsEnum> termsEnum) 
{
  shared_ptr<BytesRef> spare = make_shared<BytesRef>();
  shared_ptr<PostingsEnum> postingsEnum = nullptr;
  for (int i = 0; i < outerInstance->terms->size(); i++) {
    if (termsEnum->seekExact(
            outerInstance->terms->get(outerInstance->ords[i], spare))) {
      postingsEnum = termsEnum->postings(postingsEnum, PostingsEnum::NONE);
      float score = outerInstance->scores[outerInstance->ords[i]];
      for (int doc = postingsEnum->nextDoc();
           doc != DocIdSetIterator::NO_MORE_DOCS;
           doc = postingsEnum->nextDoc()) {
        // I prefer this:
        /*if (scores[doc] < score) {
          scores[doc] = score;
          matchingDocs.set(doc);
        }*/
        // But this behaves the same as MVInnerScorer and only then the tests
        // will pass:
        if (!matchingDocs->get(doc)) {
          scores[doc] = score;
          matchingDocs->set(doc);
        }
      }
    }
  }
}
} // namespace org::apache::lucene::search::join