using namespace std;

#include "ShardSearchingTestBase.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Directory = org::apache::lucene::store::Directory;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using PrintStreamInfoStream = org::apache::lucene::util::PrintStreamInfoStream;
using TestUtil = org::apache::lucene::util::TestUtil;

ShardSearchingTestBase::SearcherExpiredException::SearcherExpiredException(
    const wstring &message)
    : RuntimeException(message)
{
}

ShardSearchingTestBase::FieldAndShardVersion::FieldAndShardVersion(
    int nodeID, int64_t version, const wstring &field)
    : version(version), nodeID(nodeID), field(field)
{
}

int ShardSearchingTestBase::FieldAndShardVersion::hashCode()
{
  return static_cast<int>(version * nodeID + field.hashCode());
}

bool ShardSearchingTestBase::FieldAndShardVersion::equals(any _other)
{
  if (!(std::dynamic_pointer_cast<FieldAndShardVersion>(_other) != nullptr)) {
    return false;
  }

  shared_ptr<FieldAndShardVersion> *const other =
      any_cast<std::shared_ptr<FieldAndShardVersion>>(_other);

  return field == other->field && version == other->version &&
         nodeID == other->nodeID;
}

wstring ShardSearchingTestBase::FieldAndShardVersion::toString()
{
  return L"FieldAndShardVersion(field=" + field + L" nodeID=" +
         to_wstring(nodeID) + L" version=" + to_wstring(version) + L")";
}

ShardSearchingTestBase::TermAndShardVersion::TermAndShardVersion(
    int nodeID, int64_t version, shared_ptr<Term> term)
    : version(version), nodeID(nodeID), term(term)
{
}

int ShardSearchingTestBase::TermAndShardVersion::hashCode()
{
  return static_cast<int>(version * nodeID + term->hashCode());
}

bool ShardSearchingTestBase::TermAndShardVersion::equals(any _other)
{
  if (!(std::dynamic_pointer_cast<TermAndShardVersion>(_other) != nullptr)) {
    return false;
  }

  shared_ptr<TermAndShardVersion> *const other =
      any_cast<std::shared_ptr<TermAndShardVersion>>(_other);

  return term->equals(other->term) && version == other->version &&
         nodeID == other->nodeID;
}

void ShardSearchingTestBase::broadcastNodeReopen(
    int nodeID, int64_t version,
    shared_ptr<IndexSearcher> newSearcher) 
{

  if (VERBOSE) {
    wcout << L"REOPEN: nodeID=" << nodeID << L" version=" << version
          << L" maxDoc=" << newSearcher->getIndexReader()->maxDoc() << endl;
  }

  // Broadcast new collection stats for this node to all
  // other nodes:
  for (auto field : fieldsToShare) {
    shared_ptr<CollectionStatistics> *const stats =
        newSearcher->collectionStatistics(field);
    for (auto node : nodes) {
      // Don't put my own collection stats into the cache;
      // we pull locally:
      if (node->myNodeID != nodeID) {
        node->collectionStatsCache.put(
            make_shared<FieldAndShardVersion>(nodeID, version, field), stats);
      }
    }
  }
  for (auto node : nodes) {
    node->updateNodeVersion(nodeID, version);
  }
}

shared_ptr<TopDocs> ShardSearchingTestBase::searchNode(
    int nodeID, std::deque<int64_t> &nodeVersions, shared_ptr<Query> q,
    shared_ptr<Sort> sort, int numHits,
    shared_ptr<ScoreDoc> searchAfter) 
{
  shared_ptr<NodeState::ShardIndexSearcher> *const s =
      nodes[nodeID]->acquire(nodeVersions);
  try {
    if (sort == nullptr) {
      if (searchAfter != nullptr) {
        return s->localSearchAfter(searchAfter, q, numHits);
      } else {
        return s->localSearch(q, numHits);
      }
    } else {
      assert(searchAfter == nullptr); // not supported yet
      return s->localSearch(q, numHits, sort);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    nodes[nodeID]->release(s);
  }
}

unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermStatistics>>
ShardSearchingTestBase::getNodeTermStats(
    shared_ptr<Set<std::shared_ptr<Term>>> terms, int nodeID,
    int64_t version) 
{
  shared_ptr<NodeState> *const node = nodes[nodeID];
  const unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermStatistics>>
      stats = unordered_map<std::shared_ptr<Term>,
                            std::shared_ptr<TermStatistics>>();
  shared_ptr<IndexSearcher> *const s = node->searchers->acquire(version);
  if (s == nullptr) {
    throw make_shared<SearcherExpiredException>(
        L"node=" + to_wstring(nodeID) + L" version=" + to_wstring(version));
  }
  try {
    for (auto term : terms) {
      shared_ptr<TermContext> *const termContext =
          TermContext::build(s->getIndexReader()->getContext(), term);
      stats.emplace(term, s->termStatistics(term, termContext));
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    node->searchers->release(s);
  }
  return stats;
}

ShardSearchingTestBase::NodeState::ShardIndexSearcher::ShardIndexSearcher(
    shared_ptr<ShardSearchingTestBase::NodeState> outerInstance,
    std::deque<int64_t> &nodeVersions, shared_ptr<IndexReader> localReader,
    int nodeID)
    : IndexSearcher(localReader), nodeVersions(nodeVersions), myNodeID(nodeID),
      outerInstance(outerInstance)
{
  assert((myNodeID == outerInstance->myNodeID,
          L"myNodeID=" + to_wstring(nodeID) + L" NodeState.this.myNodeID=" +
              to_wstring(outerInstance->myNodeID)));
}

shared_ptr<Query>
ShardSearchingTestBase::NodeState::ShardIndexSearcher::rewrite(
    shared_ptr<Query> original) 
{
  shared_ptr<IndexSearcher> *const localSearcher =
      make_shared<IndexSearcher>(getIndexReader());
  original = localSearcher->rewrite(original);
  shared_ptr<Weight> *const weight =
      localSearcher->createWeight(original, true, 1);
  shared_ptr<Set<std::shared_ptr<Term>>> *const terms =
      unordered_set<std::shared_ptr<Term>>();
  weight->extractTerms(terms);

  // Make a single request to remote nodes for term
  // stats:
  for (int nodeID = 0; nodeID < nodeVersions.size(); nodeID++) {
    if (nodeID == myNodeID) {
      continue;
    }

    shared_ptr<Set<std::shared_ptr<Term>>> *const missing =
        unordered_set<std::shared_ptr<Term>>();
    for (auto term : terms) {
      shared_ptr<TermAndShardVersion> *const key =
          make_shared<TermAndShardVersion>(nodeID, nodeVersions[nodeID], term);
      if (outerInstance->termStatsCache.find(key) ==
          outerInstance->termStatsCache.end()) {
        missing->add(term);
      }
    }
    if (missing->size() != 0) {
      for (shared_ptr<unordered_map::Entry<std::shared_ptr<Term>,
                                           std::shared_ptr<TermStatistics>>>
               ent :
           outerInstance->outerInstance
               ->getNodeTermStats(missing, nodeID, nodeVersions[nodeID])
               .entrySet()) {
        shared_ptr<TermAndShardVersion> *const key =
            make_shared<TermAndShardVersion>(nodeID, nodeVersions[nodeID],
                                             ent.first);
        outerInstance->termStatsCache.emplace(key, ent.second);
      }
    }
  }

  return weight->getQuery();
}

shared_ptr<TermStatistics>
ShardSearchingTestBase::NodeState::ShardIndexSearcher::termStatistics(
    shared_ptr<Term> term, shared_ptr<TermContext> context) 
{
  assert(term != nullptr);
  int64_t docFreq = 0;
  int64_t totalTermFreq = 0;
  for (int nodeID = 0; nodeID < nodeVersions.size(); nodeID++) {

    shared_ptr<TermStatistics> *const subStats;
    if (nodeID == myNodeID) {
      subStats = IndexSearcher::termStatistics(term, context);
    } else {
      shared_ptr<TermAndShardVersion> *const key =
          make_shared<TermAndShardVersion>(nodeID, nodeVersions[nodeID], term);
      subStats = outerInstance->termStatsCache[key];
      // We pre-cached during rewrite so all terms
      // better be here...
      assert(subStats != nullptr);
    }

    int64_t nodeDocFreq = subStats->docFreq();
    if (docFreq >= 0 && nodeDocFreq >= 0) {
      docFreq += nodeDocFreq;
    } else {
      docFreq = -1;
    }

    int64_t nodeTotalTermFreq = subStats->totalTermFreq();
    if (totalTermFreq >= 0 && nodeTotalTermFreq >= 0) {
      totalTermFreq += nodeTotalTermFreq;
    } else {
      totalTermFreq = -1;
    }
  }

  return make_shared<TermStatistics>(term->bytes(), docFreq, totalTermFreq);
}

shared_ptr<CollectionStatistics>
ShardSearchingTestBase::NodeState::ShardIndexSearcher::collectionStatistics(
    const wstring &field) 
{
  // TODO: we could compute this on init and cache,
  // since we are re-inited whenever any nodes have a
  // new reader
  int64_t docCount = 0;
  int64_t sumTotalTermFreq = 0;
  int64_t sumDocFreq = 0;
  int64_t maxDoc = 0;

  for (int nodeID = 0; nodeID < nodeVersions.size(); nodeID++) {
    shared_ptr<FieldAndShardVersion> *const key =
        make_shared<FieldAndShardVersion>(nodeID, nodeVersions[nodeID], field);
    shared_ptr<CollectionStatistics> *const nodeStats;
    if (nodeID == myNodeID) {
      nodeStats = IndexSearcher::collectionStatistics(field);
    } else {
      nodeStats = outerInstance->collectionStatsCache[key];
    }
    if (nodeStats == nullptr) {
      wcout << L"coll stats myNodeID=" << myNodeID << L": "
            << outerInstance->collectionStatsCache.keySet() << endl;
    }
    // deque stats are pre-shared on reopen, so,
    // we better not have a cache miss:
    assert((nodeStats != nullptr,
            L"myNodeID=" + to_wstring(myNodeID) + L" nodeID=" +
                to_wstring(nodeID) + L" version=" +
                to_wstring(nodeVersions[nodeID]) + L" field=" + field));

    int64_t nodeDocCount = nodeStats->docCount();
    if (docCount >= 0 && nodeDocCount >= 0) {
      docCount += nodeDocCount;
    } else {
      docCount = -1;
    }

    int64_t nodeSumTotalTermFreq = nodeStats->sumTotalTermFreq();
    if (sumTotalTermFreq >= 0 && nodeSumTotalTermFreq >= 0) {
      sumTotalTermFreq += nodeSumTotalTermFreq;
    } else {
      sumTotalTermFreq = -1;
    }

    int64_t nodeSumDocFreq = nodeStats->sumDocFreq();
    if (sumDocFreq >= 0 && nodeSumDocFreq >= 0) {
      sumDocFreq += nodeSumDocFreq;
    } else {
      sumDocFreq = -1;
    }

    assert(nodeStats->maxDoc() >= 0);
    maxDoc += nodeStats->maxDoc();
  }

  return make_shared<CollectionStatistics>(field, maxDoc, docCount,
                                           sumTotalTermFreq, sumDocFreq);
}

shared_ptr<TopDocs>
ShardSearchingTestBase::NodeState::ShardIndexSearcher::search(
    shared_ptr<Query> query, int numHits) 
{
  std::deque<std::shared_ptr<TopDocs>> shardHits(nodeVersions.size());
  for (int nodeID = 0; nodeID < nodeVersions.size(); nodeID++) {
    if (nodeID == myNodeID) {
      // My node; run using local shard searcher we
      // already aquired:
      shardHits[nodeID] = localSearch(query, numHits);
    } else {
      shardHits[nodeID] = outerInstance->outerInstance->searchNode(
          nodeID, nodeVersions, query, nullptr, numHits, nullptr);
    }
  }

  // Merge:
  return TopDocs::merge(numHits, shardHits);
}

shared_ptr<TopDocs>
ShardSearchingTestBase::NodeState::ShardIndexSearcher::localSearch(
    shared_ptr<Query> query, int numHits) 
{
  return IndexSearcher::search(query, numHits);
}

shared_ptr<TopDocs>
ShardSearchingTestBase::NodeState::ShardIndexSearcher::searchAfter(
    shared_ptr<ScoreDoc> after, shared_ptr<Query> query,
    int numHits) 
{
  if (after == nullptr) {
    return IndexSearcher::searchAfter(after, query, numHits);
  }
  std::deque<std::shared_ptr<TopDocs>> shardHits(nodeVersions.size());
  // results are merged in that order: score, shardIndex, doc. therefore we set
  // after to after.score and depending on the nodeID we set doc to either:
  // - not collect any more documents with that score (only with worse score)
  // - collect more documents with that score (and worse) following the last
  // collected document
  // - collect all documents with that score (and worse)
  shared_ptr<ScoreDoc> shardAfter =
      make_shared<ScoreDoc>(after->doc, after->score);
  for (int nodeID = 0; nodeID < nodeVersions.size(); nodeID++) {
    if (nodeID < after->shardIndex) {
      // all documents with after.score were already collected, so collect
      // only documents with worse scores.
      shared_ptr<NodeState::ShardIndexSearcher> *const s =
          outerInstance->outerInstance->nodes[nodeID].acquire(nodeVersions);
      try {
        // Setting after.doc to reader.maxDoc-1 is a way to tell
        // TopScoreDocCollector that no more docs with that score should
        // be collected. note that in practice the shard which sends the
        // request to a remote shard won't have reader.maxDoc at hand, so
        // it will send some arbitrary value which will be fixed on the
        // other end.
        shardAfter->doc = s->getIndexReader()->maxDoc() - 1;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        outerInstance->outerInstance->nodes[nodeID].release(s);
      }
    } else if (nodeID == after->shardIndex) {
      // collect all documents following the last collected doc with
      // after.score + documents with worse scores.
      shardAfter->doc = after->doc;
    } else {
      // all documents with after.score (and worse) should be collected
      // because they didn't make it to top-N in the previous round.
      shardAfter->doc = -1;
    }
    if (nodeID == myNodeID) {
      // My node; run using local shard searcher we
      // already aquired:
      shardHits[nodeID] = localSearchAfter(shardAfter, query, numHits);
    } else {
      shardHits[nodeID] = outerInstance->outerInstance->searchNode(
          nodeID, nodeVersions, query, nullptr, numHits, shardAfter);
    }
    // System.out.println("  node=" + nodeID + " totHits=" +
    // shardHits[nodeID].totalHits);
  }

  // Merge:
  return TopDocs::merge(numHits, shardHits);
}

shared_ptr<TopDocs>
ShardSearchingTestBase::NodeState::ShardIndexSearcher::localSearchAfter(
    shared_ptr<ScoreDoc> after, shared_ptr<Query> query,
    int numHits) 
{
  return IndexSearcher::searchAfter(after, query, numHits);
}

shared_ptr<TopFieldDocs>
ShardSearchingTestBase::NodeState::ShardIndexSearcher::search(
    shared_ptr<Query> query, int numHits,
    shared_ptr<Sort> sort) 
{
  assert(sort != nullptr);
  std::deque<std::shared_ptr<TopFieldDocs>> shardHits(nodeVersions.size());
  for (int nodeID = 0; nodeID < nodeVersions.size(); nodeID++) {
    if (nodeID == myNodeID) {
      // My node; run using local shard searcher we
      // already aquired:
      shardHits[nodeID] = localSearch(query, numHits, sort);
    } else {
      shardHits[nodeID] = std::static_pointer_cast<TopFieldDocs>(
          outerInstance->outerInstance->searchNode(nodeID, nodeVersions, query,
                                                   sort, numHits, nullptr));
    }
  }

  // Merge:
  return TopDocs::merge(sort, numHits, shardHits);
}

shared_ptr<TopFieldDocs>
ShardSearchingTestBase::NodeState::ShardIndexSearcher::localSearch(
    shared_ptr<Query> query, int numHits,
    shared_ptr<Sort> sort) 
{
  return IndexSearcher::search(query, numHits, sort);
}

ShardSearchingTestBase::NodeState::NodeState(
    shared_ptr<ShardSearchingTestBase> outerInstance, shared_ptr<Random> random,
    int nodeID, int numNodes) 
    : dir(LuceneTestCase::newFSDirectory(
          LuceneTestCase::createTempDir(L"ShardSearchingTestBase"))),
      writer(make_shared<IndexWriter>(dir, iwc)),
      searchers(make_shared<SearcherLifetimeManager>()),
      mgr(make_shared<SearcherManager>(writer, nullptr)), myNodeID(nodeID),
      currentNodeVersions(std::deque<int64_t>(numNodes)),
      outerInstance(outerInstance)
{
  // TODO: set warmer
  shared_ptr<MockAnalyzer> analyzer =
      make_shared<MockAnalyzer>(ShardSearchingTestBase::random());
  analyzer->setMaxTokenLength(TestUtil::nextInt(
      ShardSearchingTestBase::random(), 1, IndexWriter::MAX_TERM_LENGTH));
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(analyzer);
  iwc->setOpenMode(IndexWriterConfig::OpenMode::CREATE);
  if (VERBOSE) {
    iwc->setInfoStream(make_shared<PrintStreamInfoStream>(System::out));
  }

  // Init w/ 0s... caller above will do initial
  // "broadcast" by calling initSearcher:
}

void ShardSearchingTestBase::NodeState::initSearcher(
    std::deque<int64_t> &nodeVersions) 
{
  assert(currentShardSearcher == nullptr);
  System::arraycopy(nodeVersions, 0, currentNodeVersions, 0,
                    currentNodeVersions.size());
  currentShardSearcher = make_shared<ShardIndexSearcher>(
      shared_from_this(), currentNodeVersions.clone(),
      mgr->acquire()->getIndexReader(), myNodeID);
}

void ShardSearchingTestBase::NodeState::updateNodeVersion(
    int nodeID, int64_t version) 
{
  currentNodeVersions[nodeID] = version;
  if (currentShardSearcher != nullptr) {
    currentShardSearcher->getIndexReader()->decRef();
  }
  currentShardSearcher = make_shared<ShardIndexSearcher>(
      shared_from_this(), currentNodeVersions.clone(),
      mgr->acquire()->getIndexReader(), myNodeID);
}

shared_ptr<ShardIndexSearcher> ShardSearchingTestBase::NodeState::acquire()
{
  while (true) {
    shared_ptr<ShardIndexSearcher> *const s = currentShardSearcher;
    // In theory the reader could get decRef'd to 0
    // before we have a chance to incRef, ie if a reopen
    // happens right after the above line, this thread
    // gets stalled, and the old IR is closed.  So we
    // must try/retry until incRef succeeds:
    if (s->getIndexReader()->tryIncRef()) {
      return s;
    }
  }
}

void ShardSearchingTestBase::NodeState::release(
    shared_ptr<ShardIndexSearcher> s) 
{
  s->getIndexReader()->decRef();
}

shared_ptr<ShardIndexSearcher>
ShardSearchingTestBase::NodeState::acquire(std::deque<int64_t> &nodeVersions)
{
  shared_ptr<IndexSearcher> *const s =
      searchers->acquire(nodeVersions[myNodeID]);
  if (s == nullptr) {
    throw make_shared<SearcherExpiredException>(
        L"nodeID=" + to_wstring(myNodeID) + L" version=" +
        to_wstring(nodeVersions[myNodeID]));
  }
  return make_shared<ShardIndexSearcher>(shared_from_this(), nodeVersions,
                                         s->getIndexReader(), myNodeID);
}

void ShardSearchingTestBase::NodeState::reopen() 
{
  shared_ptr<IndexSearcher> *const before = mgr->acquire();
  mgr->release(before);

  mgr->maybeRefresh();
  shared_ptr<IndexSearcher> *const after = mgr->acquire();
  try {
    if (after != before) {
      // New searcher was opened
      constexpr int64_t version = searchers->record(after);
      searchers->prune(make_shared<SearcherLifetimeManager::PruneByAge>(
          outerInstance->maxSearcherAgeSeconds));
      outerInstance->broadcastNodeReopen(myNodeID, version, after);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    mgr->release(after);
  }
}

ShardSearchingTestBase::NodeState::~NodeState()
{
  if (currentShardSearcher != nullptr) {
    currentShardSearcher->getIndexReader()->decRef();
  }
  delete searchers;
  delete mgr;
  delete writer;
  delete dir;
}

ShardSearchingTestBase::ChangeIndices::ChangeIndices(
    shared_ptr<ShardSearchingTestBase> outerInstance)
    : outerInstance(outerInstance)
{
}

void ShardSearchingTestBase::ChangeIndices::run()
{
  try {
    shared_ptr<LineFileDocs> *const docs =
        make_shared<LineFileDocs>(LuceneTestCase::random());
    int numDocs = 0;
    while (System::nanoTime() < outerInstance->endTimeNanos) {
      constexpr int what = LuceneTestCase::random()->nextInt(3);
      shared_ptr<NodeState> *const node =
          outerInstance->nodes[LuceneTestCase::random()->nextInt(
              outerInstance->nodes.size())];
      if (numDocs == 0 || what == 0) {
        node->writer->addDocument(docs->nextDoc());
        numDocs++;
      } else if (what == 1) {
        node->writer->updateDocument(
            make_shared<Term>(L"docid",
                              L"" + LuceneTestCase::random()->nextInt(numDocs)),
            docs->nextDoc());
        numDocs++;
      } else {
        node->writer->deleteDocuments({make_shared<Term>(
            L"docid", L"" + LuceneTestCase::random()->nextInt(numDocs))});
      }
      // TODO: doc blocks too

      if (LuceneTestCase::random()->nextInt(17) == 12) {
        node->writer->commit();
      }

      if (LuceneTestCase::random()->nextInt(17) == 12) {
        outerInstance
            ->nodes[LuceneTestCase::random()->nextInt(
                outerInstance->nodes.size())]
            ->reopen();
      }
    }
  } catch (const runtime_error &t) {
    wcout << L"FAILED:" << endl;
    t.printStackTrace(System::out);
    throw runtime_error(t);
  }
}

void ShardSearchingTestBase::start(int numNodes, double runTimeSec,
                                   int maxSearcherAgeSeconds) 
{

  endTimeNanos =
      System::nanoTime() + static_cast<int64_t>(runTimeSec * 1000000000);
  this->maxSearcherAgeSeconds = maxSearcherAgeSeconds;

  nodes = std::deque<std::shared_ptr<NodeState>>(numNodes);
  for (int nodeID = 0; nodeID < numNodes; nodeID++) {
    nodes[nodeID] =
        make_shared<NodeState>(shared_from_this(), random(), nodeID, numNodes);
  }

  std::deque<int64_t> nodeVersions(nodes.size());
  for (int nodeID = 0; nodeID < numNodes; nodeID++) {
    shared_ptr<IndexSearcher> *const s = nodes[nodeID]->mgr.acquire();
    try {
      nodeVersions[nodeID] = nodes[nodeID]->searchers.record(s);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      nodes[nodeID]->mgr.release(s);
    }
  }

  for (int nodeID = 0; nodeID < numNodes; nodeID++) {
    shared_ptr<IndexSearcher> *const s = nodes[nodeID]->mgr.acquire();
    assert(nodeVersions[nodeID] == nodes[nodeID]->searchers.record(s));
    assert(s != nullptr);
    try {
      broadcastNodeReopen(nodeID, nodeVersions[nodeID], s);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      nodes[nodeID]->mgr.release(s);
    }
  }

  changeIndicesThread = make_shared<ChangeIndices>(shared_from_this());
  changeIndicesThread->start();
}

void ShardSearchingTestBase::finish() 
{
  changeIndicesThread->join();
  for (auto node : nodes) {
    node->close();
  }
}

ShardSearchingTestBase::SearcherAndVersion::SearcherAndVersion(
    shared_ptr<IndexSearcher> searcher, int64_t version)
    : searcher(searcher), version(version)
{
}
} // namespace org::apache::lucene::search