#pragma once
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/LineFileDocs.h"

#include  "core/src/java/org/apache/lucene/index/DocumentsWriterFlushControl.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/LiveIndexWriterConfig.h"
#include  "core/src/java/org/apache/lucene/index/DocumentsWriterPerThreadPool.h"
#include  "core/src/java/org/apache/lucene/index/ThreadState.h"

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

using ThreadState =
    org::apache::lucene::index::DocumentsWriterPerThreadPool::ThreadState;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestFlushByRamOrCountsPolicy : public LuceneTestCase
{
  GET_CLASS_NAME(TestFlushByRamOrCountsPolicy)

private:
  static std::shared_ptr<LineFileDocs> lineDocFile;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

  virtual void testFlushByRam() ;

  virtual void testFlushByRamLargeBuffer() throw(IOException,
                                                 InterruptedException);

protected:
  virtual void runFlushByRam(int numThreads, double maxRamMB,
                             bool ensureNotStalled) throw(IOException,
                                                          InterruptedException);

public:
  virtual void testFlushDocCount() ;

  virtual void testRandom() ;

  virtual void testStallControl() ;

protected:
  virtual void assertActiveBytesAfter(
      std::shared_ptr<DocumentsWriterFlushControl> flushControl);

public:
  class IndexThread : public Thread
  {
    GET_CLASS_NAME(IndexThread)
  public:
    std::shared_ptr<IndexWriter> writer;
    std::shared_ptr<LiveIndexWriterConfig> iwc;
    std::shared_ptr<LineFileDocs> docs;

  private:
    std::shared_ptr<AtomicInteger> pendingDocs;
    const bool doRandomCommit;

  public:
    IndexThread(std::shared_ptr<AtomicInteger> pendingDocs, int numThreads,
                std::shared_ptr<IndexWriter> writer,
                std::shared_ptr<LineFileDocs> docs, bool doRandomCommit);

    void run() override;

  protected:
    std::shared_ptr<IndexThread> shared_from_this()
    {
      return std::static_pointer_cast<IndexThread>(Thread::shared_from_this());
    }
  };

private:
  class MockDefaultFlushPolicy : public FlushByRamOrCountsPolicy
  {
    GET_CLASS_NAME(MockDefaultFlushPolicy)
  public:
    int64_t peakBytesWithoutFlush = std::numeric_limits<int>::min();
    int64_t peakDocCountWithoutFlush = std::numeric_limits<int>::min();
    bool hasMarkedPending = false;

    void onDelete(std::shared_ptr<DocumentsWriterFlushControl> control,
                  std::shared_ptr<ThreadState> state) override;

    void onInsert(std::shared_ptr<DocumentsWriterFlushControl> control,
                  std::shared_ptr<ThreadState> state) override;

  protected:
    std::shared_ptr<MockDefaultFlushPolicy> shared_from_this()
    {
      return std::static_pointer_cast<MockDefaultFlushPolicy>(
          FlushByRamOrCountsPolicy::shared_from_this());
    }
  };

public:
  static void
  findPending(std::shared_ptr<DocumentsWriterFlushControl> flushControl,
              std::deque<std::shared_ptr<ThreadState>> &pending,
              std::deque<std::shared_ptr<ThreadState>> &notPending);

protected:
  std::shared_ptr<TestFlushByRamOrCountsPolicy> shared_from_this()
  {
    return std::static_pointer_cast<TestFlushByRamOrCountsPolicy>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
