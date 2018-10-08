#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexDeletionPolicy;
}

namespace org::apache::lucene::index
{
class IndexWriterConfig;
}
namespace org::apache::lucene::index
{
class IndexCommit;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::index
{
class SnapshotDeletionPolicy;
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
namespace org::apache::lucene::index
{

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

//
// This was developed for Lucene In Action,
// http://lucenebook.com
//

class TestSnapshotDeletionPolicy : public LuceneTestCase
{
  GET_CLASS_NAME(TestSnapshotDeletionPolicy)
public:
  static const std::wstring INDEX_PATH;

protected:
  virtual std::shared_ptr<IndexWriterConfig>
  getConfig(std::shared_ptr<Random> random,
            std::shared_ptr<IndexDeletionPolicy> dp);

  virtual void
  checkSnapshotExists(std::shared_ptr<Directory> dir,
                      std::shared_ptr<IndexCommit> c) ;

  virtual void checkMaxDoc(std::shared_ptr<IndexCommit> commit,
                           int expectedMaxDoc) ;

  std::deque<std::shared_ptr<IndexCommit>> snapshots =
      std::deque<std::shared_ptr<IndexCommit>>();

  virtual void
  prepareIndexAndSnapshots(std::shared_ptr<SnapshotDeletionPolicy> sdp,
                           std::shared_ptr<IndexWriter> writer,
                           int numSnapshots) throw(std::runtime_error,
                                                   IOException);

  virtual std::shared_ptr<SnapshotDeletionPolicy>
  getDeletionPolicy() ;

  virtual void
  assertSnapshotExists(std::shared_ptr<Directory> dir,
                       std::shared_ptr<SnapshotDeletionPolicy> sdp,
                       int numSnapshots,
                       bool checkIndexCommitSame) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSnapshotDeletionPolicy() throws
  // Exception
  virtual void testSnapshotDeletionPolicy() ;

private:
  void runTest(std::shared_ptr<Random> random,
               std::shared_ptr<Directory> dir) ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestSnapshotDeletionPolicy> outerInstance;

    int64_t stopTime = 0;
    std::shared_ptr<org::apache::lucene::index::IndexWriter> writer;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestSnapshotDeletionPolicy> outerInstance,
        int64_t stopTime,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> writer);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  /**
   * Example showing how to use the SnapshotDeletionPolicy to take a backup.
   * This method does not really do a backup; instead, it reads every byte of
   * every file just to test that the files indeed exist and are readable even
   * while the index is changing.
   */
public:
  virtual void backupIndex(
      std::shared_ptr<Directory> dir,
      std::shared_ptr<SnapshotDeletionPolicy> dp) ;

private:
  void copyFiles(std::shared_ptr<Directory> dir,
                 std::shared_ptr<IndexCommit> cp) ;

public:
  std::deque<char> buffer = std::deque<char>(4096);

private:
  void readFile(std::shared_ptr<Directory> dir,
                const std::wstring &name) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBasicSnapshots() throws Exception
  virtual void testBasicSnapshots() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMultiThreadedSnapshotting() throws
  // Exception
  virtual void testMultiThreadedSnapshotting() ;

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<TestSnapshotDeletionPolicy> outerInstance;

    std::shared_ptr<org::apache::lucene::index::IndexWriter> writer;
    std::shared_ptr<org::apache::lucene::index::SnapshotDeletionPolicy> sdp;
    std::deque<std::shared_ptr<org::apache::lucene::index::IndexCommit>>
        snapshots;
    std::shared_ptr<CountDownLatch> startingGun;
    int finalI = 0;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<TestSnapshotDeletionPolicy> outerInstance,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> writer,
        std::shared_ptr<org::apache::lucene::index::SnapshotDeletionPolicy> sdp,
        std::deque<std::shared_ptr<org::apache::lucene::index::IndexCommit>>
            &snapshots,
        std::shared_ptr<CountDownLatch> startingGun, int finalI);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRollbackToOldSnapshot() throws
  // Exception
  virtual void testRollbackToOldSnapshot() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testReleaseSnapshot() throws Exception
  virtual void testReleaseSnapshot() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSnapshotLastCommitTwice() throws
  // Exception
  virtual void testSnapshotLastCommitTwice() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMissingCommits() throws Exception
  virtual void testMissingCommits() ;

protected:
  std::shared_ptr<TestSnapshotDeletionPolicy> shared_from_this()
  {
    return std::static_pointer_cast<TestSnapshotDeletionPolicy>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
