using namespace std;

#include "FacetsCollector.h"

namespace org::apache::lucene::facet
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Collector = org::apache::lucene::search::Collector;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MultiCollector = org::apache::lucene::search::MultiCollector;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using Sort = org::apache::lucene::search::Sort;
using TopDocs = org::apache::lucene::search::TopDocs;
using TopDocsCollector = org::apache::lucene::search::TopDocsCollector;
using TopFieldCollector = org::apache::lucene::search::TopFieldCollector;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using TopScoreDocCollector = org::apache::lucene::search::TopScoreDocCollector;
using TotalHitCountCollector =
    org::apache::lucene::search::TotalHitCountCollector;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;

FacetsCollector::MatchingDocs::MatchingDocs(
    shared_ptr<LeafReaderContext> context, shared_ptr<DocIdSet> bits,
    int totalHits, std::deque<float> &scores)
    : context(context), bits(bits), scores(scores), totalHits(totalHits)
{
}

FacetsCollector::FacetsCollector() : FacetsCollector(false) {}

FacetsCollector::FacetsCollector(bool keepScores) : keepScores(keepScores) {}

bool FacetsCollector::getKeepScores() { return keepScores; }

deque<std::shared_ptr<MatchingDocs>> FacetsCollector::getMatchingDocs()
{
  if (docsBuilder != nullptr) {
    matchingDocs.push_back(make_shared<MatchingDocs>(
        this->context, docsBuilder->build(), totalHits, scores));
    docsBuilder.reset();
    scores.clear();
    context.reset();
  }

  return matchingDocs;
}

void FacetsCollector::collect(int doc) 
{
  docsBuilder->grow(1)->add(doc);
  if (keepScores) {
    if (totalHits >= scores.size()) {
      std::deque<float> newScores(ArrayUtil::oversize(totalHits + 1, 4));
      System::arraycopy(scores, 0, newScores, 0, totalHits);
      scores = newScores;
    }
    scores[totalHits] = scorer->score();
  }
  totalHits++;
}

bool FacetsCollector::needsScores() { return true; }

void FacetsCollector::setScorer(shared_ptr<Scorer> scorer) 
{
  this->scorer = scorer;
}

void FacetsCollector::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  if (docsBuilder != nullptr) {
    matchingDocs.push_back(make_shared<MatchingDocs>(
        this->context, docsBuilder->build(), totalHits, scores));
  }
  docsBuilder = make_shared<DocIdSetBuilder>(context->reader()->maxDoc());
  totalHits = 0;
  if (keepScores) {
    scores = std::deque<float>(64); // some initial size
  }
  this->context = context;
}

shared_ptr<TopDocs>
FacetsCollector::search(shared_ptr<IndexSearcher> searcher, shared_ptr<Query> q,
                        int n, shared_ptr<Collector> fc) 
{
  return doSearch(searcher, nullptr, q, n, nullptr, false, false, fc);
}

shared_ptr<TopFieldDocs>
FacetsCollector::search(shared_ptr<IndexSearcher> searcher, shared_ptr<Query> q,
                        int n, shared_ptr<Sort> sort,
                        shared_ptr<Collector> fc) 
{
  if (sort == nullptr) {
    throw invalid_argument(L"sort must not be null");
  }
  return std::static_pointer_cast<TopFieldDocs>(
      doSearch(searcher, nullptr, q, n, sort, false, false, fc));
}

shared_ptr<TopFieldDocs>
FacetsCollector::search(shared_ptr<IndexSearcher> searcher, shared_ptr<Query> q,
                        int n, shared_ptr<Sort> sort, bool doDocScores,
                        bool doMaxScore,
                        shared_ptr<Collector> fc) 
{
  if (sort == nullptr) {
    throw invalid_argument(L"sort must not be null");
  }
  return std::static_pointer_cast<TopFieldDocs>(
      doSearch(searcher, nullptr, q, n, sort, doDocScores, doMaxScore, fc));
}

shared_ptr<TopDocs>
FacetsCollector::searchAfter(shared_ptr<IndexSearcher> searcher,
                             shared_ptr<ScoreDoc> after, shared_ptr<Query> q,
                             int n, shared_ptr<Collector> fc) 
{
  return doSearch(searcher, after, q, n, nullptr, false, false, fc);
}

shared_ptr<TopDocs>
FacetsCollector::searchAfter(shared_ptr<IndexSearcher> searcher,
                             shared_ptr<ScoreDoc> after, shared_ptr<Query> q,
                             int n, shared_ptr<Sort> sort,
                             shared_ptr<Collector> fc) 
{
  if (sort == nullptr) {
    throw invalid_argument(L"sort must not be null");
  }
  return doSearch(searcher, after, q, n, sort, false, false, fc);
}

shared_ptr<TopDocs> FacetsCollector::searchAfter(
    shared_ptr<IndexSearcher> searcher, shared_ptr<ScoreDoc> after,
    shared_ptr<Query> q, int n, shared_ptr<Sort> sort, bool doDocScores,
    bool doMaxScore, shared_ptr<Collector> fc) 
{
  if (sort == nullptr) {
    throw invalid_argument(L"sort must not be null");
  }
  return doSearch(searcher, after, q, n, sort, doDocScores, doMaxScore, fc);
}

shared_ptr<TopDocs> FacetsCollector::doSearch(
    shared_ptr<IndexSearcher> searcher, shared_ptr<ScoreDoc> after,
    shared_ptr<Query> q, int n, shared_ptr<Sort> sort, bool doDocScores,
    bool doMaxScore, shared_ptr<Collector> fc) 
{

  int limit = searcher->getIndexReader()->maxDoc();
  if (limit == 0) {
    limit = 1;
  }
  n = min(n, limit);

  if (after != nullptr && after->doc >= limit) {
    throw invalid_argument(
        L"after.doc exceeds the number of documents in the reader: after.doc=" +
        to_wstring(after->doc) + L" limit=" + to_wstring(limit));
  }

  shared_ptr<TopDocs> topDocs = nullptr;
  if (n == 0) {
    shared_ptr<TotalHitCountCollector> totalHitCountCollector =
        make_shared<TotalHitCountCollector>();
    searcher->search(q, MultiCollector::wrap({totalHitCountCollector, fc}));
    topDocs =
        make_shared<TopDocs>(totalHitCountCollector->getTotalHits(),
                             std::deque<std::shared_ptr<ScoreDoc>>(0), NAN);
  } else {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: org.apache.lucene.search.TopDocsCollector<?>
    // hitsCollector;
    shared_ptr < TopDocsCollector < ? >> hitsCollector;
    if (sort != nullptr) {
      if (after != nullptr &&
          !(std::dynamic_pointer_cast<FieldDoc>(after) != nullptr)) {
        // TODO: if we fix type safety of TopFieldDocs we can
        // remove this
        throw invalid_argument(L"after must be a FieldDoc; got " + after);
      }
      bool fillFields = true;
      hitsCollector = TopFieldCollector::create(
          sort, n, std::static_pointer_cast<FieldDoc>(after), fillFields,
          doDocScores, doMaxScore,
          true); // TODO: can we disable exact hit counts
    } else {
      hitsCollector = TopScoreDocCollector::create(n, after);
    }
    searcher->search(q, MultiCollector::wrap({hitsCollector, fc}));

    topDocs = hitsCollector->topDocs();
  }
  return topDocs;
}
} // namespace org::apache::lucene::facet