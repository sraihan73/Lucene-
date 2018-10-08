#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::replicator::nrt
{
class NodeProcess;
}

namespace org::apache::lucene::replicator::nrt
{
class SimpleTransLog;
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

namespace org::apache::lucene::replicator::nrt
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/*
  TODO
    - fangs
      - sometimes have one replica be really slow at copying / have random
  pauses (fake GC) / etc.
    - test should not print scary exceptions and then succeed!
    - are the pre-copied-completed-merged files not being cleared in primary?
      - hmm the logic isn't right today?  a replica may skip pulling a given
  copy state, that recorded the finished merged segments?
    - later
      - since all nodes are local, we could have a different test only impl that
  just does local file copies instead of via tcp...
      - get all other "single shard" functions working too: this cluster should
  "act like" a single shard
        - SLM
        - controlled nrt reopen thread / returning long gen on write
        - live field values
      - add indexes
      - replica should also track maxSegmentName its seen, and tap into
  inflateGens if it's later promoted to primary?
      - if we named index files using segment's ID we wouldn't have file name
  conflicts after primary crash / rollback?
      - back pressure on indexing if replicas can't keep up?
      - get xlog working on top?  needs to be checkpointed, so we can correlate
  IW ops to NRT reader version and prune xlog based on commit quorum
        - maybe fix IW to return "gen" or "seq id" or "segment name" or
  something?
      - replica can copy files from other replicas too / use multicast / rsync /
  something
      - each replica could also pre-open a SegmentReader after pre-copy when
  warming a merge
      - we can pre-copy newly flushed files too, for cases where reopen rate is
  low vs IW's flushing because RAM buffer is full
      - opto: pre-copy files as they are written; if they will become CFS, we
  can build CFS on the replica?
      - what about multiple commit points?
      - fix primary to init directly from an open replica, instead of having to
  commit/close the replica first
*/

// Tricky cases:
//   - we are pre-copying a merge, then replica starts up part way through, so
//   it misses that pre-copy and must do it on next nrt point
//   - a down replica starts up, but it's "from the future" vs the current
//   primary, and must copy over file names with different contents
//     but referenced by its latest commit point, so it must fully remove that
//     commit ... which is a hazardous window
//   - replica comes up just as the primary is crashing / moving
//   - electing a new primary when a replica is just finishing its nrt sync: we
//   need to wait for it so we are sure to get the "most up to
//     date" replica
//   - replica comes up after merged segment finished so it doesn't copy over
//   the merged segment "promptly" (i.e. only sees it on NRT refresh)

/**
 * Test case showing how to implement NRT replication.  This test spawns a
 * sub-process per-node, running TestNRTReplicationChild.
 *
 * One node is primary, and segments are periodically flushed there, then
 * concurrently the N replica nodes copy the new files over and open new
 * readers, while primary also opens a new reader.
 *
 * Nodes randomly crash and are restarted.  If the primary crashes, a replica is
 * promoted.
 *
 * Merges are currently first finished on the primary and then pre-copied out to
 * replicas with a merged segment warmer so they don't block ongoing NRT
 * reopens.  Probably replicas could do their own merging instead, but this is
 * more complex and may not be better overall (merging takes a lot of IO
 * resources).
 *
 * Slow network is simulated with a RateLimiter.
 */

// MockRandom's .sd file has no index header/footer:
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({"MockRandom", "Memory", "Direct",
// "SimpleText"}) @SuppressSysoutChecks(bugUrl = "Stuff gets printed, important
// stuff for debugging a failure") public class TestStressNRTReplication extends
// org.apache.lucene.util.LuceneTestCase
class TestStressNRTReplication : public LuceneTestCase
{

  // Test evilness controls:

  /** Randomly crash the current primary (losing data!) and promote the "next
   * best" replica. */
public:
  static constexpr bool DO_CRASH_PRIMARY = true;

  /** Randomly crash (JVM core dumps) a replica; it will later randomly be
   * restarted and sync itself. */
  static constexpr bool DO_CRASH_REPLICA = true;

  /** Randomly gracefully close a replica; it will later be restarted and sync
   * itself. */
  static constexpr bool DO_CLOSE_REPLICA = true;

  /** Randomly gracefully close the primary; it will later be restarted and sync
   * itself. */
  static constexpr bool DO_CLOSE_PRIMARY = true;

  /** If false, all child + parent output is interleaved into single stdout/err
   */
  static constexpr bool SEPARATE_CHILD_OUTPUT = false;

  /** Randomly crash whole cluster and then restart it */
  static constexpr bool DO_FULL_CLUSTER_CRASH = true;

  /** True if we randomly flip a bit while copying files out */
  static constexpr bool DO_BIT_FLIPS_DURING_COPY = true;

  /** Set to a non-null value to force exactly that many nodes; else, it's
   * random. */
  static const std::optional<int> NUM_NODES;

  const std::shared_ptr<AtomicBoolean> failed =
      std::make_shared<AtomicBoolean>();

  const std::shared_ptr<AtomicBoolean> stop = std::make_shared<AtomicBoolean>();

  /** cwd where we start each child (server) node */
private:
  std::shared_ptr<Path> childTempDir;

public:
  int64_t primaryGen = 0;

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile long lastPrimaryVersion;
  int64_t lastPrimaryVersion = 0;

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile NodeProcess primary;
  std::shared_ptr<NodeProcess> primary;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile NodeProcess[] nodes;
  std::deque<std::shared_ptr<NodeProcess>> nodes;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile long[] nodeTimeStamps;
  std::deque<int64_t> nodeTimeStamps;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile bool[] starting;
  std::deque<bool> starting;

  std::deque<std::shared_ptr<Path>> indexPaths;

  std::shared_ptr<Path> transLogPath;
  std::shared_ptr<SimpleTransLog> transLog;
  const std::shared_ptr<AtomicInteger> markerUpto =
      std::make_shared<AtomicInteger>();
  const std::shared_ptr<AtomicInteger> markerID =
      std::make_shared<AtomicInteger>();

  /** Maps searcher version to how many hits the query body:the matched. */
  const std::unordered_map<int64_t, int> hitCounts =
      std::make_shared<ConcurrentHashMap<int64_t, int>>();

  /** Maps searcher version to how many marker documents matched.  This should
   * only ever grow (we never delete marker documents). */
  const std::unordered_map<int64_t, int> versionToMarker =
      std::make_shared<ConcurrentHashMap<int64_t, int>>();

  /** Maps searcher version to xlog location when refresh of this version
   * started. */
  const std::unordered_map<int64_t, int64_t> versionToTransLogLocation =
      std::make_shared<ConcurrentHashMap<int64_t, int64_t>>();

  const std::shared_ptr<AtomicLong> nodeStartCounter =
      std::make_shared<AtomicLong>();

  const std::shared_ptr<Set<int>> crashingNodes =
      Collections::synchronizedSet(std::unordered_set<int>());

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void test() throws Exception
  virtual void test() ;

private:
  bool anyNodesStarting();

  /** Picks a replica and promotes it as new primary. */
  void promoteReplica() ;

public:
  virtual void startPrimary(int id) ;

  /** Launches a child "server" (separate JVM), which is either primary or
   * replica node */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "ProcessBuilder requires
  // java.io.File for CWD") NodeProcess startNode(final int id,
  // java.nio.file.Path indexPath, bool isPrimary, long forcePrimaryVersion)
  // throws java.io.IOException
  virtual std::shared_ptr<NodeProcess>
  startNode(int const id, std::shared_ptr<Path> indexPath, bool isPrimary,
            int64_t forcePrimaryVersion) ;

private:
  void nodeClosed(int id);

  /** Sends currently alive replicas to primary, which uses this to know who to
   * notify when it does a refresh */
  void sendReplicasToPrimary();

public:
  virtual void addVersionMarker(int64_t version, int count);

  virtual void addTransLogLoc(int64_t version, int64_t loc);

  // Periodically wakes up and starts up any down nodes:
private:
  class RestartThread : public Thread
  {
    GET_CLASS_NAME(RestartThread)
  private:
    std::shared_ptr<TestStressNRTReplication> outerInstance;

  public:
    RestartThread(std::shared_ptr<TestStressNRTReplication> outerInstance);

    void run() override;

  private:
    class ThreadAnonymousInnerClass : public Thread
    {
      GET_CLASS_NAME(ThreadAnonymousInnerClass)
    private:
      std::shared_ptr<RestartThread> outerInstance;

      std::deque<std::shared_ptr<Thread>> startupThreads;
      int idx = 0;

    public:
      ThreadAnonymousInnerClass(
          std::shared_ptr<RestartThread> outerInstance,
          std::deque<std::shared_ptr<Thread>> &startupThreads, int idx);

      void run() override;

    protected:
      std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ThreadAnonymousInnerClass>(
            Thread::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<RestartThread> shared_from_this()
    {
      return std::static_pointer_cast<RestartThread>(
          Thread::shared_from_this());
    }
  };

  /** Randomly picks a node and runs a search against it */
private:
  class SearchThread : public Thread
  {
    GET_CLASS_NAME(SearchThread)
  private:
    std::shared_ptr<TestStressNRTReplication> outerInstance;

  public:
    SearchThread(std::shared_ptr<TestStressNRTReplication> outerInstance);

    void run() override;

  protected:
    std::shared_ptr<SearchThread> shared_from_this()
    {
      return std::static_pointer_cast<SearchThread>(Thread::shared_from_this());
    }
  };

private:
  class IndexThread : public Thread
  {
    GET_CLASS_NAME(IndexThread)
  private:
    std::shared_ptr<TestStressNRTReplication> outerInstance;

  public:
    IndexThread(std::shared_ptr<TestStressNRTReplication> outerInstance);

    void run() override;

  protected:
    std::shared_ptr<IndexThread> shared_from_this()
    {
      return std::static_pointer_cast<IndexThread>(Thread::shared_from_this());
    }
  };

public:
  static void message(const std::wstring &message);

  static void message(const std::wstring &message, int64_t localStartNS);

protected:
  std::shared_ptr<TestStressNRTReplication> shared_from_this()
  {
    return std::static_pointer_cast<TestStressNRTReplication>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::replicator::nrt
