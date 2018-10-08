#pragma once
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Term;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class ScoreDoc;
}
namespace org::apache::lucene::search
{
class Sort;
}
namespace org::apache::lucene::search
{
class TopDocs;
}
namespace org::apache::lucene::search
{
class TermStatistics;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::search
{
class SearcherLifetimeManager;
}
namespace org::apache::lucene::search
{
class SearcherManager;
}
namespace org::apache::lucene::search
{
class CollectionStatistics;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class TermContext;
}
namespace org::apache::lucene::search
{
class TopFieldDocs;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::search
{

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// TODO
//   - doc blocks?  so we can test joins/grouping...
//   - controlled consistency (NRTMgr)

/**
 * Base test class for simulating distributed search across multiple shards.
 */
class ShardSearchingTestBase : public LuceneTestCase
{
  GET_CLASS_NAME(ShardSearchingTestBase)

  // TODO: maybe SLM should throw this instead of returning null...
  /**
   * Thrown when the lease for a searcher has expired.
   */
public:
  class SearcherExpiredException : public std::runtime_error
  {
    GET_CLASS_NAME(SearcherExpiredException)
  public:
    SearcherExpiredException(const std::wstring &message);

  protected:
    std::shared_ptr<SearcherExpiredException> shared_from_this()
    {
      return std::static_pointer_cast<SearcherExpiredException>(
          RuntimeException::shared_from_this());
    }
  };

private:
  class FieldAndShardVersion
      : public std::enable_shared_from_this<FieldAndShardVersion>
  {
    GET_CLASS_NAME(FieldAndShardVersion)
  private:
    const int64_t version;
    const int nodeID;
    const std::wstring field;

  public:
    FieldAndShardVersion(int nodeID, int64_t version,
                         const std::wstring &field);

    virtual int hashCode();

    bool equals(std::any _other) override;

    virtual std::wstring toString();
  };

private:
  class TermAndShardVersion
      : public std::enable_shared_from_this<TermAndShardVersion>
  {
    GET_CLASS_NAME(TermAndShardVersion)
  private:
    const int64_t version;
    const int nodeID;
    const std::shared_ptr<Term> term;

  public:
    TermAndShardVersion(int nodeID, int64_t version,
                        std::shared_ptr<Term> term);

    virtual int hashCode();

    bool equals(std::any _other) override;
  };

  // We share collection stats for these fields on each node
  // reopen:
private:
  std::deque<std::wstring> const fieldsToShare =
      std::deque<std::wstring>{L"body", L"title"};

  // Called by one node once it has reopened, to notify all
  // other nodes.  This is just a mock (since it goes and
  // directly updates all other nodes, in RAM)... in a real
  // env this would hit the wire, sending version &
  // collection stats to all other nodes:
public:
  virtual void broadcastNodeReopen(
      int nodeID, int64_t version,
      std::shared_ptr<IndexSearcher> newSearcher) ;

  // TODO: broadcastNodeExpire?  then we can purge the
  // known-stale cache entries...

  // MOCK: in a real env you have to hit the wire
  // (send this query to all remote nodes
  // concurrently):
  virtual std::shared_ptr<TopDocs>
  searchNode(int nodeID, std::deque<int64_t> &nodeVersions,
             std::shared_ptr<Query> q, std::shared_ptr<Sort> sort, int numHits,
             std::shared_ptr<ScoreDoc> searchAfter) ;

  // Mock: in a real env, this would hit the wire and get
  // term stats from remote node
  virtual std::unordered_map<std::shared_ptr<Term>,
                             std::shared_ptr<TermStatistics>>
  getNodeTermStats(std::shared_ptr<Set<std::shared_ptr<Term>>> terms,
                   int nodeID, int64_t version) ;

protected:
  class NodeState final : public std::enable_shared_from_this<NodeState>
  {
    GET_CLASS_NAME(NodeState)
  private:
    std::shared_ptr<ShardSearchingTestBase> outerInstance;

  public:
    const std::shared_ptr<Directory> dir;
    const std::shared_ptr<IndexWriter> writer;
    const std::shared_ptr<SearcherLifetimeManager> searchers;
    const std::shared_ptr<SearcherManager> mgr;
    const int myNodeID;
    std::deque<int64_t> const currentNodeVersions;

    // TODO: nothing evicts from here!!!  Somehow, on searcher
    // expiration on remote nodes we must evict from our
    // local cache...?  And still LRU otherwise (for the
    // still-live searchers).

  private:
    const std::unordered_map<std::shared_ptr<FieldAndShardVersion>,
                             std::shared_ptr<CollectionStatistics>>
        collectionStatsCache = std::make_shared<
            ConcurrentHashMap<std::shared_ptr<FieldAndShardVersion>,
                              std::shared_ptr<CollectionStatistics>>>();
    const std::unordered_map<std::shared_ptr<TermAndShardVersion>,
                             std::shared_ptr<TermStatistics>>
        termStatsCache = std::make_shared<
            ConcurrentHashMap<std::shared_ptr<TermAndShardVersion>,
                              std::shared_ptr<TermStatistics>>>();

    /** Matches docs in the local shard but scores based on
     *  aggregated stats ("mock distributed scoring") from all
     *  nodes. */

  public:
    class ShardIndexSearcher : public IndexSearcher
    {
      GET_CLASS_NAME(ShardIndexSearcher)
    private:
      std::shared_ptr<ShardSearchingTestBase::NodeState> outerInstance;

      // Version for the node searchers we search:
    public:
      std::deque<int64_t> const nodeVersions;
      const int myNodeID;

      ShardIndexSearcher(
          std::shared_ptr<ShardSearchingTestBase::NodeState> outerInstance,
          std::deque<int64_t> &nodeVersions,
          std::shared_ptr<IndexReader> localReader, int nodeID);

      std::shared_ptr<Query>
      rewrite(std::shared_ptr<Query> original)  override;

      std::shared_ptr<TermStatistics> termStatistics(
          std::shared_ptr<Term> term,
          std::shared_ptr<TermContext> context)  override;

      std::shared_ptr<CollectionStatistics> collectionStatistics(
          const std::wstring &field)  override;

      std::shared_ptr<TopDocs> search(std::shared_ptr<Query> query,
                                      int numHits)  override;

      virtual std::shared_ptr<TopDocs>
      localSearch(std::shared_ptr<Query> query, int numHits) ;

      std::shared_ptr<TopDocs>
      searchAfter(std::shared_ptr<ScoreDoc> after, std::shared_ptr<Query> query,
                  int numHits)  override;

      virtual std::shared_ptr<TopDocs>
      localSearchAfter(std::shared_ptr<ScoreDoc> after,
                       std::shared_ptr<Query> query,
                       int numHits) ;

      std::shared_ptr<TopFieldDocs>
      search(std::shared_ptr<Query> query, int numHits,
             std::shared_ptr<Sort> sort)  override;

      virtual std::shared_ptr<TopFieldDocs>
      localSearch(std::shared_ptr<Query> query, int numHits,
                  std::shared_ptr<Sort> sort) ;

    protected:
      std::shared_ptr<ShardIndexSearcher> shared_from_this()
      {
        return std::static_pointer_cast<ShardIndexSearcher>(
            IndexSearcher::shared_from_this());
      }
    };

  private:
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: private volatile ShardIndexSearcher currentShardSearcher;
    std::shared_ptr<ShardIndexSearcher> currentShardSearcher;

  public:
    NodeState(std::shared_ptr<ShardSearchingTestBase> outerInstance,
              std::shared_ptr<Random> random, int nodeID,
              int numNodes) ;

    void initSearcher(std::deque<int64_t> &nodeVersions) ;

    void updateNodeVersion(int nodeID, int64_t version) ;

    // Get the current (fresh) searcher for this node
    std::shared_ptr<ShardIndexSearcher> acquire();

    void release(std::shared_ptr<ShardIndexSearcher> s) ;

    // Get and old searcher matching the specified versions:
    std::shared_ptr<ShardIndexSearcher>
    acquire(std::deque<int64_t> &nodeVersions);

    // Reopen local reader
    void reopen() ;

    virtual ~NodeState();
  };

  // TODO: make this more realistic, ie, each node should
  // have its own thread, so we have true node to node
  // concurrency
private:
  class ChangeIndices final : public Thread
  {
    GET_CLASS_NAME(ChangeIndices)
  private:
    std::shared_ptr<ShardSearchingTestBase> outerInstance;

  public:
    ChangeIndices(std::shared_ptr<ShardSearchingTestBase> outerInstance);

    void run() override;

  protected:
    std::shared_ptr<ChangeIndices> shared_from_this()
    {
      return std::static_pointer_cast<ChangeIndices>(
          Thread::shared_from_this());
    }
  };

protected:
  std::deque<std::shared_ptr<NodeState>> nodes;

public:
  int maxSearcherAgeSeconds = 0;
  int64_t endTimeNanos = 0;

private:
  std::shared_ptr<Thread> changeIndicesThread;

protected:
  virtual void start(int numNodes, double runTimeSec,
                     int maxSearcherAgeSeconds) ;

  virtual void finish() ;

  /**
   * An IndexSearcher and associated version (lease)
   */
protected:
  class SearcherAndVersion
      : public std::enable_shared_from_this<SearcherAndVersion>
  {
    GET_CLASS_NAME(SearcherAndVersion)
  public:
    const std::shared_ptr<IndexSearcher> searcher;
    const int64_t version;

    SearcherAndVersion(std::shared_ptr<IndexSearcher> searcher,
                       int64_t version);
  };

protected:
  std::shared_ptr<ShardSearchingTestBase> shared_from_this()
  {
    return std::static_pointer_cast<ShardSearchingTestBase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
