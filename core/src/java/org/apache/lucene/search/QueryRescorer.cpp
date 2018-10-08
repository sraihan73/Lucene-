using namespace std;

#include "QueryRescorer.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

QueryRescorer::QueryRescorer(shared_ptr<Query> query) : query(query) {}

shared_ptr<TopDocs> QueryRescorer::rescore(shared_ptr<IndexSearcher> searcher,
                                           shared_ptr<TopDocs> firstPassTopDocs,
                                           int topN) 
{
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      firstPassTopDocs->scoreDocs.clone();
  Arrays::sort(hits,
               make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

  deque<std::shared_ptr<LeafReaderContext>> leaves =
      searcher->getIndexReader()->leaves();

  shared_ptr<Query> rewritten = searcher->rewrite(query);
  shared_ptr<Weight> weight = searcher->createWeight(rewritten, true, 1);

  // Now merge sort docIDs from hits, with reader's leaves:
  int hitUpto = 0;
  int readerUpto = -1;
  int endDoc = 0;
  int docBase = 0;
  shared_ptr<Scorer> scorer = nullptr;

  while (hitUpto < hits.size()) {
    shared_ptr<ScoreDoc> hit = hits[hitUpto];
    int docID = hit->doc;
    shared_ptr<LeafReaderContext> readerContext = nullptr;
    while (docID >= endDoc) {
      readerUpto++;
      readerContext = leaves[readerUpto];
      endDoc = readerContext->docBase + readerContext->reader()->maxDoc();
    }

    if (readerContext != nullptr) {
      // We advanced to another segment:
      docBase = readerContext->docBase;
      scorer = weight->scorer(readerContext);
    }

    if (scorer != nullptr) {
      int targetDoc = docID - docBase;
      int actualDoc = scorer->docID();
      if (actualDoc < targetDoc) {
        actualDoc = scorer->begin().advance(targetDoc);
      }

      if (actualDoc == targetDoc) {
        // Query did match this doc:
        hit->score = combine(hit->score, true, scorer->score());
      } else {
        // Query did not match this doc:
        assert(actualDoc > targetDoc);
        hit->score = combine(hit->score, false, 0.0f);
      }
    } else {
      // Query did not match this doc:
      hit->score = combine(hit->score, false, 0.0f);
    }

    hitUpto++;
  }

  // TODO: we should do a partial sort (of only topN)
  // instead, but typically the number of hits is
  // smallish:
  Arrays::sort(hits,
               make_shared<ComparatorAnonymousInnerClass2>(shared_from_this()));

  if (topN < hits.size()) {
    std::deque<std::shared_ptr<ScoreDoc>> subset(topN);
    System::arraycopy(hits, 0, subset, 0, topN);
    hits = subset;
  }

  return make_shared<TopDocs>(firstPassTopDocs->totalHits, hits,
                              hits[0]->score);
}

QueryRescorer::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<QueryRescorer> outerInstance)
{
  this->outerInstance = outerInstance;
}

int QueryRescorer::ComparatorAnonymousInnerClass::compare(
    shared_ptr<ScoreDoc> a, shared_ptr<ScoreDoc> b)
{
  return a->doc - b->doc;
}

QueryRescorer::ComparatorAnonymousInnerClass2::ComparatorAnonymousInnerClass2(
    shared_ptr<QueryRescorer> outerInstance)
{
  this->outerInstance = outerInstance;
}

int QueryRescorer::ComparatorAnonymousInnerClass2::compare(
    shared_ptr<ScoreDoc> a, shared_ptr<ScoreDoc> b)
{
  // Sort by score descending, then docID ascending:
  if (a->score > b->score) {
    return -1;
  } else if (a->score < b->score) {
    return 1;
  } else {
    // This subtraction can't overflow int
    // because docIDs are >= 0:
    return a->doc - b->doc;
  }
}

shared_ptr<Explanation>
QueryRescorer::explain(shared_ptr<IndexSearcher> searcher,
                       shared_ptr<Explanation> firstPassExplanation,
                       int docID) 
{
  shared_ptr<Explanation> secondPassExplanation =
      searcher->explain(query, docID);

  optional<float> secondPassScore = secondPassExplanation->isMatch()
                                        ? secondPassExplanation->getValue()
                                        : nullopt;

  float score;
  if (!secondPassScore) {
    score = combine(firstPassExplanation->getValue(), false, 0.0f);
  } else {
    score = combine(firstPassExplanation->getValue(), true,
                    secondPassScore.value());
  }

  shared_ptr<Explanation> first =
      Explanation::match(firstPassExplanation->getValue(), L"first pass score",
                         firstPassExplanation);

  shared_ptr<Explanation> second;
  if (!secondPassScore) {
    second = Explanation::noMatch(L"no second pass score");
  } else {
    second = Explanation::match(secondPassScore, L"second pass score",
                                secondPassExplanation);
  }

  return Explanation::match(
      score, L"combined first and second pass score using " + getClass(),
      {first, second});
}

shared_ptr<TopDocs> QueryRescorer::rescore(shared_ptr<IndexSearcher> searcher,
                                           shared_ptr<TopDocs> topDocs,
                                           shared_ptr<Query> query,
                                           double const weight,
                                           int topN) 
{
  return make_shared<QueryRescorerAnonymousInnerClass>(query, weight)
      .rescore(searcher, topDocs, topN);
}

QueryRescorer::QueryRescorerAnonymousInnerClass::
    QueryRescorerAnonymousInnerClass(
        shared_ptr<org::apache::lucene::search::Query> query, double weight)
    : QueryRescorer(query)
{
  this->weight = weight;
}

float QueryRescorer::QueryRescorerAnonymousInnerClass::combine(
    float firstPassScore, bool secondPassMatches, float secondPassScore)
{
  float score = firstPassScore;
  if (secondPassMatches) {
    score += weight * secondPassScore;
  }
  return score;
}
} // namespace org::apache::lucene::search