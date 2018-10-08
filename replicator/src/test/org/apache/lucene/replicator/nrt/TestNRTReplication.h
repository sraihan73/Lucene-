#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class LineFileDocs;
}

namespace org::apache::lucene::replicator::nrt
{
class NodeProcess;
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

using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// MockRandom's .sd file has no index header/footer:
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({"MockRandom", "Memory", "Direct",
// "SimpleText"}) @SuppressSysoutChecks(bugUrl = "Stuff gets printed, important
// stuff for debugging a failure") public class TestNRTReplication extends
// org.apache.lucene.util.LuceneTestCase
class TestNRTReplication : public LuceneTestCase
{

  /** cwd where we start each child (server) node */
private:
  std::shared_ptr<Path> childTempDir;

public:
  const std::shared_ptr<AtomicLong> nodeStartCounter =
      std::make_shared<AtomicLong>();

private:
  int64_t nextPrimaryGen = 0;
  int64_t lastPrimaryGen = 0;

public:
  std::shared_ptr<LineFileDocs> docs;

  /** Launches a child "server" (separate JVM), which is either primary or
   * replica node */
private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "ProcessBuilder requires
  // java.io.File for CWD") private NodeProcess startNode(int primaryTCPPort,
  // final int id, java.nio.file.Path indexPath, long forcePrimaryVersion,
  // bool willCrash) throws java.io.IOException
  std::shared_ptr<NodeProcess> startNode(int primaryTCPPort, int const id,
                                         std::shared_ptr<Path> indexPath,
                                         int64_t forcePrimaryVersion,
                                         bool willCrash) ;

public:
  void setUp()  override;

  void tearDown()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testReplicateDeleteAllDocuments()
  // throws Exception
  virtual void testReplicateDeleteAllDocuments() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testReplicateForceMerge() throws
  // Exception
  virtual void testReplicateForceMerge() ;

  // Start up, index 10 docs, replicate, but crash and restart the replica
  // without committing it:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testReplicaCrashNoCommit() throws
  // Exception
  virtual void testReplicaCrashNoCommit() ;

  // Start up, index 10 docs, replicate, commit, crash and restart the replica
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testReplicaCrashWithCommit() throws
  // Exception
  virtual void testReplicaCrashWithCommit() ;

  // Start up, index 10 docs, replicate, commit, crash, index more docs,
  // replicate, then restart the replica
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testIndexingWhileReplicaIsDown() throws
  // Exception
  virtual void testIndexingWhileReplicaIsDown() ;

  // Crash primary and promote a replica
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testCrashPrimary1() throws Exception
  virtual void testCrashPrimary1() ;

  // Crash primary and then restart it
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testCrashPrimary2() throws Exception
  virtual void testCrashPrimary2() ;

  // Crash primary and then restart it, while a replica node is down, then bring
  // replica node back up and make sure it properly "unforks" itself
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testCrashPrimary3() throws Exception
  virtual void testCrashPrimary3() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testCrashPrimaryWhileCopying() throws
  // Exception
  virtual void testCrashPrimaryWhileCopying() ;

private:
  void
  assertWriteLockHeld(std::shared_ptr<Path> path) ;

public:
  virtual void testCrashReplica() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testFullClusterCrash() throws Exception
  virtual void testFullClusterCrash() ;

  /** Tell primary current replicas. */
private:
  void
  sendReplicasToPrimary(std::shared_ptr<NodeProcess> primary,
                        std::deque<NodeProcess> &replicas) ;

  /** Verifies this node is currently searching the specified version with the
   * specified total hit count, or that it eventually does when keepTrying is
   * true. */
  void assertVersionAndHits(std::shared_ptr<NodeProcess> node,
                            int64_t expectedVersion,
                            int expectedHitCount) ;

  void waitForVersionAndHits(std::shared_ptr<NodeProcess> node,
                             int64_t expectedVersion,
                             int expectedHitCount) ;

public:
  static void message(const std::wstring &message);

protected:
  std::shared_ptr<TestNRTReplication> shared_from_this()
  {
    return std::static_pointer_cast<TestNRTReplication>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::replicator::nrt
