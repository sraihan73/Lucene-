using namespace std;

#include "IndexSearcher.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiFields = org::apache::lucene::index::MultiFields;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Terms = org::apache::lucene::index::Terms;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using NIOFSDirectory = org::apache::lucene::store::NIOFSDirectory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;
const shared_ptr<org::apache::lucene::search::similarities::Similarity>
    IndexSearcher::NON_SCORING_SIMILARITY =
        make_shared<SimilarityAnonymousInnerClass>();

IndexSearcher::SimilarityAnonymousInnerClass::SimilarityAnonymousInnerClass() {}

int64_t IndexSearcher::SimilarityAnonymousInnerClass::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  throw make_shared<UnsupportedOperationException>(
      L"This Similarity may only be used for searching, not indexing");
}

shared_ptr<Similarity::SimWeight>
IndexSearcher::SimilarityAnonymousInnerClass::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  return make_shared<SimWeightAnonymousInnerClass>(shared_from_this());
}

IndexSearcher::SimilarityAnonymousInnerClass::SimWeightAnonymousInnerClass::
    SimWeightAnonymousInnerClass(
        shared_ptr<SimilarityAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Similarity::SimScorer>
IndexSearcher::SimilarityAnonymousInnerClass::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<SimScorerAnonymousInnerClass>(shared_from_this());
}

IndexSearcher::SimilarityAnonymousInnerClass::SimScorerAnonymousInnerClass::
    SimScorerAnonymousInnerClass(
        shared_ptr<SimilarityAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

float IndexSearcher::SimilarityAnonymousInnerClass::
    SimScorerAnonymousInnerClass::score(int doc, float freq)
{
  return 0.0f;
}

float IndexSearcher::SimilarityAnonymousInnerClass::
    SimScorerAnonymousInnerClass::computeSlopFactor(int distance)
{
  return 1.0f;
}

float IndexSearcher::SimilarityAnonymousInnerClass::
    SimScorerAnonymousInnerClass::computePayloadFactor(
        int doc, int start, int end, shared_ptr<BytesRef> payload)
{
  return 1.0f;
}

shared_ptr<QueryCache> IndexSearcher::DEFAULT_QUERY_CACHE;
shared_ptr<QueryCachingPolicy> IndexSearcher::DEFAULT_CACHING_POLICY =
    make_shared<UsageTrackingQueryCachingPolicy>();

IndexSearcher::StaticConstructor::StaticConstructor()
{
  constexpr int maxCachedQueries = 1000;
  // min of 32MB or 5% of the heap size
  constexpr int64_t maxRamBytesUsed =
      min(1LL << 25, Runtime::getRuntime().maxMemory() / 20);
  DEFAULT_QUERY_CACHE =
      make_shared<LRUQueryCache>(maxCachedQueries, maxRamBytesUsed);
}

IndexSearcher::StaticConstructor IndexSearcher::staticConstructor;
const shared_ptr<org::apache::lucene::search::similarities::Similarity>
    IndexSearcher::defaultSimilarity = make_shared<
        org::apache::lucene::search::similarities::BM25Similarity>();

shared_ptr<Similarity> IndexSearcher::getDefaultSimilarity()
{
  return defaultSimilarity;
}

shared_ptr<QueryCache> IndexSearcher::getDefaultQueryCache()
{
  return DEFAULT_QUERY_CACHE;
}

void IndexSearcher::setDefaultQueryCache(
    shared_ptr<QueryCache> defaultQueryCache)
{
  DEFAULT_QUERY_CACHE = defaultQueryCache;
}

shared_ptr<QueryCachingPolicy> IndexSearcher::getDefaultQueryCachingPolicy()
{
  return DEFAULT_CACHING_POLICY;
}

void IndexSearcher::setDefaultQueryCachingPolicy(
    shared_ptr<QueryCachingPolicy> defaultQueryCachingPolicy)
{
  DEFAULT_CACHING_POLICY = defaultQueryCachingPolicy;
}

IndexSearcher::IndexSearcher(shared_ptr<IndexReader> r)
    : IndexSearcher(r, nullptr)
{
}

IndexSearcher::IndexSearcher(shared_ptr<IndexReader> r,
                             shared_ptr<ExecutorService> executor)
    : IndexSearcher(r->getContext(), executor)
{
}

IndexSearcher::IndexSearcher(shared_ptr<IndexReaderContext> context,
                             shared_ptr<ExecutorService> executor)
    : reader(context->reader()), readerContext(context),
      leafContexts(context->leaves()),
      leafSlices(executor == nullptr ? nullptr : slices(leafContexts)),
      executor(executor)
{
  assert((context->isTopLevel,
          L"IndexSearcher's ReaderContext must be topLevel for reader" +
              context->reader()));
}

IndexSearcher::IndexSearcher(shared_ptr<IndexReaderContext> context)
    : IndexSearcher(context, nullptr)
{
}

void IndexSearcher::setQueryCache(shared_ptr<QueryCache> queryCache)
{
  this->queryCache = queryCache;
}

shared_ptr<QueryCache> IndexSearcher::getQueryCache() { return queryCache; }

void IndexSearcher::setQueryCachingPolicy(
    shared_ptr<QueryCachingPolicy> queryCachingPolicy)
{
  this->queryCachingPolicy = Objects::requireNonNull(queryCachingPolicy);
}

shared_ptr<QueryCachingPolicy> IndexSearcher::getQueryCachingPolicy()
{
  return queryCachingPolicy;
}

std::deque<std::shared_ptr<LeafSlice>>
IndexSearcher::slices(deque<std::shared_ptr<LeafReaderContext>> &leaves)
{
  std::deque<std::shared_ptr<LeafSlice>> slices(leaves.size());
  for (int i = 0; i < slices.size(); i++) {
    slices[i] = make_shared<LeafSlice>(leaves[i]);
  }
  return slices;
}

shared_ptr<IndexReader> IndexSearcher::getIndexReader() { return reader; }

shared_ptr<Document> IndexSearcher::doc(int docID) 
{
  return reader->document(docID);
}

void IndexSearcher::doc(
    int docID, shared_ptr<StoredFieldVisitor> fieldVisitor) 
{
  reader->document(docID, fieldVisitor);
}

shared_ptr<Document>
IndexSearcher::doc(int docID,
                   shared_ptr<Set<wstring>> fieldsToLoad) 
{
  return reader->document(docID, fieldsToLoad);
}

void IndexSearcher::setSimilarity(shared_ptr<Similarity> similarity)
{
  this->similarity = similarity;
}

shared_ptr<Similarity> IndexSearcher::getSimilarity(bool needsScores)
{
  return needsScores ? similarity : NON_SCORING_SIMILARITY;
}

int IndexSearcher::count(shared_ptr<Query> query) 
{
  query = rewrite(query);
  while (true) {
    // remove wrappers that don't matter for counts
    if (std::dynamic_pointer_cast<ConstantScoreQuery>(query) != nullptr) {
      query = (std::static_pointer_cast<ConstantScoreQuery>(query))->getQuery();
    } else {
      break;
    }
  }

  // some counts can be computed in constant time
  if (std::dynamic_pointer_cast<MatchAllDocsQuery>(query) != nullptr) {
    return reader->numDocs();
  } else if (std::dynamic_pointer_cast<TermQuery>(query) != nullptr &&
             reader->hasDeletions() == false) {
    shared_ptr<Term> term =
        (std::static_pointer_cast<TermQuery>(query))->getTerm();
    int count = 0;
    for (auto leaf : reader->leaves()) {
      count += leaf->reader()->docFreq(term);
    }
    return count;
  }

  // general case: create a collecor and count matches
  shared_ptr<CollectorManager<std::shared_ptr<TotalHitCountCollector>, int>>
      *const collectorManager =
          make_shared<CollectorManagerAnonymousInnerClass>(shared_from_this());
  return search(query, collectorManager);
}

IndexSearcher::CollectorManagerAnonymousInnerClass::
    CollectorManagerAnonymousInnerClass(shared_ptr<IndexSearcher> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TotalHitCountCollector>
IndexSearcher::CollectorManagerAnonymousInnerClass::newCollector() throw(
    IOException)
{
  return make_shared<TotalHitCountCollector>();
}

optional<int> IndexSearcher::CollectorManagerAnonymousInnerClass::reduce(
    shared_ptr<deque<std::shared_ptr<TotalHitCountCollector>>>
        collectors) 
{
  int total = 0;
  for (auto collector : collectors) {
    total += collector->getTotalHits();
  }
  return total;
}

shared_ptr<TopDocs> IndexSearcher::searchAfter(shared_ptr<ScoreDoc> after,
                                               shared_ptr<Query> query,
                                               int numHits) 
{
  constexpr int limit = max(1, reader->maxDoc());
  if (after != nullptr && after->doc >= limit) {
    throw invalid_argument(
        L"after.doc exceeds the number of documents in the reader: after.doc=" +
        to_wstring(after->doc) + L" limit=" + to_wstring(limit));
  }

  constexpr int cappedNumHits = min(numHits, limit);

  shared_ptr<CollectorManager<std::shared_ptr<TopScoreDocCollector>,
                              std::shared_ptr<TopDocs>>> *const manager =
      make_shared<CollectorManagerAnonymousInnerClass2>(shared_from_this(),
                                                        after, cappedNumHits);

  return search(query, manager);
}

IndexSearcher::CollectorManagerAnonymousInnerClass2::
    CollectorManagerAnonymousInnerClass2(
        shared_ptr<IndexSearcher> outerInstance,
        shared_ptr<org::apache::lucene::search::ScoreDoc> after,
        int cappedNumHits)
{
  this->outerInstance = outerInstance;
  this->after = after;
  this->cappedNumHits = cappedNumHits;
}

shared_ptr<TopScoreDocCollector>
IndexSearcher::CollectorManagerAnonymousInnerClass2::newCollector() throw(
    IOException)
{
  return TopScoreDocCollector::create(cappedNumHits, after);
}

shared_ptr<TopDocs> IndexSearcher::CollectorManagerAnonymousInnerClass2::reduce(
    shared_ptr<deque<std::shared_ptr<TopScoreDocCollector>>>
        collectors) 
{
  std::deque<std::shared_ptr<TopDocs>> topDocs(collectors->size());
  int i = 0;
  for (auto collector : collectors) {
    topDocs[i++] = collector->topDocs();
  }
  return TopDocs::merge(0, cappedNumHits, topDocs, true);
}

shared_ptr<TopDocs> IndexSearcher::search(shared_ptr<Query> query,
                                          int n) 
{
  return searchAfter(nullptr, query, n);
}

void IndexSearcher::search(shared_ptr<Query> query,
                           shared_ptr<Collector> results) 
{
  query = rewrite(query);
  search(leafContexts, createWeight(query, results->needsScores(), 1), results);
}

shared_ptr<TopFieldDocs>
IndexSearcher::search(shared_ptr<Query> query, int n, shared_ptr<Sort> sort,
                      bool doDocScores, bool doMaxScore) 
{
  return searchAfter(nullptr, query, n, sort, doDocScores, doMaxScore);
}

shared_ptr<TopFieldDocs>
IndexSearcher::search(shared_ptr<Query> query, int n,
                      shared_ptr<Sort> sort) 
{
  return searchAfter(nullptr, query, n, sort, false, false);
}

shared_ptr<TopDocs>
IndexSearcher::searchAfter(shared_ptr<ScoreDoc> after, shared_ptr<Query> query,
                           int n, shared_ptr<Sort> sort) 
{
  return searchAfter(after, query, n, sort, false, false);
}

shared_ptr<TopFieldDocs>
IndexSearcher::searchAfter(shared_ptr<ScoreDoc> after, shared_ptr<Query> query,
                           int numHits, shared_ptr<Sort> sort, bool doDocScores,
                           bool doMaxScore) 
{
  if (after != nullptr &&
      !(std::dynamic_pointer_cast<FieldDoc>(after) != nullptr)) {
    // TODO: if we fix type safety of TopFieldDocs we can
    // remove this
    throw invalid_argument(L"after must be a FieldDoc; got " + after);
  }
  return searchAfter(std::static_pointer_cast<FieldDoc>(after), query, numHits,
                     sort, doDocScores, doMaxScore);
}

shared_ptr<TopFieldDocs>
IndexSearcher::searchAfter(shared_ptr<FieldDoc> after, shared_ptr<Query> query,
                           int numHits, shared_ptr<Sort> sort, bool doDocScores,
                           bool doMaxScore) 
{
  constexpr int limit = max(1, reader->maxDoc());
  if (after != nullptr && after->doc >= limit) {
    throw invalid_argument(
        L"after.doc exceeds the number of documents in the reader: after.doc=" +
        to_wstring(after->doc) + L" limit=" + to_wstring(limit));
  }
  constexpr int cappedNumHits = min(numHits, limit);
  shared_ptr<Sort> *const rewrittenSort = sort->rewrite(shared_from_this());

  shared_ptr<CollectorManager<std::shared_ptr<TopFieldCollector>,
                              std::shared_ptr<TopFieldDocs>>> *const manager =
      make_shared<CollectorManagerAnonymousInnerClass3>(
          shared_from_this(), after, doDocScores, doMaxScore, cappedNumHits,
          rewrittenSort);

  return search(query, manager);
}

IndexSearcher::CollectorManagerAnonymousInnerClass3::
    CollectorManagerAnonymousInnerClass3(
        shared_ptr<IndexSearcher> outerInstance,
        shared_ptr<org::apache::lucene::search::FieldDoc> after,
        bool doDocScores, bool doMaxScore, int cappedNumHits,
        shared_ptr<org::apache::lucene::search::Sort> rewrittenSort)
{
  this->outerInstance = outerInstance;
  this->after = after;
  this->doDocScores = doDocScores;
  this->doMaxScore = doMaxScore;
  this->cappedNumHits = cappedNumHits;
  this->rewrittenSort = rewrittenSort;
}

shared_ptr<TopFieldCollector>
IndexSearcher::CollectorManagerAnonymousInnerClass3::newCollector() throw(
    IOException)
{
  constexpr bool fillFields = true;
  // TODO: don't pay the price for accurate hit counts by default
  return TopFieldCollector::create(rewrittenSort, cappedNumHits, after,
                                   fillFields, doDocScores, doMaxScore, true);
}

shared_ptr<TopFieldDocs>
IndexSearcher::CollectorManagerAnonymousInnerClass3::reduce(
    shared_ptr<deque<std::shared_ptr<TopFieldCollector>>>
        collectors) 
{
  std::deque<std::shared_ptr<TopFieldDocs>> topDocs(collectors->size());
  int i = 0;
  for (auto collector : collectors) {
    topDocs[i++] = collector->topDocs();
  }
  return TopDocs::merge(rewrittenSort, 0, cappedNumHits, topDocs, true);
}

template <typename C, typename T>
T IndexSearcher::search(
    shared_ptr<Query> query,
    shared_ptr<CollectorManager<C, T>> collectorManager) 
{
  static_assert(is_base_of<Collector, C>::value,
                L"C must inherit from Collector");

  if (executor == nullptr) {
    constexpr C collector = collectorManager->newCollector();
    search(query, collector);
    return collectorManager->reduce(Collections::singletonList(collector));
  } else {
    const deque<C> collectors = deque<C>(leafSlices.size());
    bool needsScores = false;
    for (int i = 0; i < leafSlices.size(); ++i) {
      constexpr C collector = collectorManager->newCollector();
      collectors.push_back(collector);
      needsScores |= collector->needsScores();
    }

    query = rewrite(query);
    shared_ptr<Weight> *const weight = createWeight(query, needsScores, 1);
    const deque<Future<C>> topDocsFutures =
        deque<Future<C>>(leafSlices.size());
    for (int i = 0; i < leafSlices.size(); ++i) {
      std::deque<std::shared_ptr<LeafReaderContext>> leaves =
          leafSlices[i]->leaves;
      constexpr C collector = collectors[i];
      topDocsFutures.push_back(
          executor->submit(make_shared<CallableAnonymousInnerClass>(
              shared_from_this(), weight, leaves, collector)));
    }

    const deque<C> collectedCollectors = deque<C>();
    for (auto future : topDocsFutures) {
      try {
        collectedCollectors.push_back(future->get());
      } catch (const InterruptedException &e) {
        throw make_shared<ThreadInterruptedException>(e);
      } catch (const ExecutionException &e) {
        throw runtime_error(e);
      }
    }

    return collectorManager->reduce(collectors);
  }
}

IndexSearcher::CallableAnonymousInnerClass::CallableAnonymousInnerClass(
    shared_ptr<IndexSearcher> outerInstance,
    shared_ptr<org::apache::lucene::search::Weight> weight,
    deque<std::shared_ptr<LeafReaderContext>> &leaves,
    shared_ptr<Collector> collector)
{
  this->outerInstance = outerInstance;
  this->weight = weight;
  this->leaves = leaves;
  this->collector = collector;
}

shared_ptr<C>
IndexSearcher::CallableAnonymousInnerClass::call() 
{
  outerInstance->search(Arrays::asList(leaves), weight, collector);
  return collector;
}

void IndexSearcher::search(deque<std::shared_ptr<LeafReaderContext>> &leaves,
                           shared_ptr<Weight> weight,
                           shared_ptr<Collector> collector) 
{

  // TODO: should we make this
  // threaded...?  the Collector could be sync'd?
  // always use single thread:
  for (auto ctx : leaves) { // search each subreader
    shared_ptr<LeafCollector> *const leafCollector;
    try {
      leafCollector = collector->getLeafCollector(ctx);
    } catch (const CollectionTerminatedException &e) {
      // there is no doc of interest in this reader context
      // continue with the following leaf
      continue;
    }
    shared_ptr<BulkScorer> scorer = weight->bulkScorer(ctx);
    if (scorer != nullptr) {
      try {
        scorer->score(leafCollector, ctx->reader()->getLiveDocs());
      } catch (const CollectionTerminatedException &e) {
        // collection was terminated prematurely
        // continue with the following leaf
      }
    }
  }
}

shared_ptr<Query>
IndexSearcher::rewrite(shared_ptr<Query> original) 
{
  shared_ptr<Query> query = original;
  for (shared_ptr<Query> rewrittenQuery = query->rewrite(reader);
       rewrittenQuery != query; rewrittenQuery = query->rewrite(reader)) {
    query = rewrittenQuery;
  }
  return query;
}

shared_ptr<Explanation> IndexSearcher::explain(shared_ptr<Query> query,
                                               int doc) 
{
  query = rewrite(query);
  return explain(createWeight(query, true, 1), doc);
}

shared_ptr<Explanation> IndexSearcher::explain(shared_ptr<Weight> weight,
                                               int doc) 
{
  int n = ReaderUtil::subIndex(doc, leafContexts);
  shared_ptr<LeafReaderContext> *const ctx = leafContexts[n];
  int deBasedDoc = doc - ctx->docBase;
  shared_ptr<Bits> *const liveDocs = ctx->reader()->getLiveDocs();
  if (liveDocs != nullptr && liveDocs->get(deBasedDoc) == false) {
    return Explanation::noMatch(L"Document " + to_wstring(doc) +
                                L" is deleted");
  }
  return weight->explain(ctx, deBasedDoc);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public Weight createNormalizedWeight(Query query,
// bool needsScores) throws java.io.IOException
shared_ptr<Weight>
IndexSearcher::createNormalizedWeight(shared_ptr<Query> query,
                                      bool needsScores) 
{
  query = rewrite(query);
  return createWeight(query, needsScores, 1.0f);
}

shared_ptr<Weight> IndexSearcher::createWeight(shared_ptr<Query> query,
                                               bool needsScores,
                                               float boost) 
{
  shared_ptr<QueryCache> *const queryCache = this->queryCache;
  shared_ptr<Weight> weight =
      query->createWeight(shared_from_this(), needsScores, boost);
  if (needsScores == false && queryCache != nullptr) {
    weight = queryCache->doCache(weight, queryCachingPolicy);
  }
  return weight;
}

shared_ptr<IndexReaderContext> IndexSearcher::getTopReaderContext()
{
  return readerContext;
}

IndexSearcher::LeafSlice::LeafSlice(deque<LeafReaderContext> &leaves)
    : leaves(leaves)
{
}

wstring IndexSearcher::toString()
{
  return L"IndexSearcher(" + reader + L"; executor=" + executor + L")";
}

shared_ptr<TermStatistics> IndexSearcher::termStatistics(
    shared_ptr<Term> term, shared_ptr<TermContext> context) 
{
  return make_shared<TermStatistics>(term->bytes(), context->docFreq(),
                                     context->totalTermFreq());
}

shared_ptr<CollectionStatistics>
IndexSearcher::collectionStatistics(const wstring &field) 
{
  constexpr int docCount;
  constexpr int64_t sumTotalTermFreq;
  constexpr int64_t sumDocFreq;

  assert(field != L"");

  shared_ptr<Terms> terms = MultiFields::getTerms(reader, field);
  if (terms == nullptr) {
    docCount = 0;
    sumTotalTermFreq = 0;
    sumDocFreq = 0;
  } else {
    docCount = terms->getDocCount();
    sumTotalTermFreq = terms->getSumTotalTermFreq();
    sumDocFreq = terms->getSumDocFreq();
  }

  return make_shared<CollectionStatistics>(field, reader->maxDoc(), docCount,
                                           sumTotalTermFreq, sumDocFreq);
}
} // namespace org::apache::lucene::search