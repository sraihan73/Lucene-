#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class BufferedUpdates;
}

namespace org::apache::lucene::index
{
class DocumentsWriterDeleteQueue;
}
namespace org::apache::lucene::index
{
class DeleteSlice;
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

using DeleteSlice =
    org::apache::lucene::index::DocumentsWriterDeleteQueue::DeleteSlice;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Unit test for {@link DocumentsWriterDeleteQueue}
 */
class TestDocumentsWriterDeleteQueue : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocumentsWriterDeleteQueue)

public:
  virtual void testUpdateDelteSlices() ;

private:
  void assertAllBetween(int start, int end,
                        std::shared_ptr<BufferedUpdates> deletes,
                        std::deque<Integer> &ids);

public:
  virtual void testClear();

  virtual void testAnyChanges() ;

  virtual void testPartiallyAppliedGlobalSlice() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestDocumentsWriterDeleteQueue> outerInstance;

    std::shared_ptr<org::apache::lucene::index::DocumentsWriterDeleteQueue>
        queue;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestDocumentsWriterDeleteQueue> outerInstance,
        std::shared_ptr<org::apache::lucene::index::DocumentsWriterDeleteQueue>
            queue);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testStressDeleteQueue() ;

private:
  class UpdateThread : public Thread
  {
    GET_CLASS_NAME(UpdateThread)
  public:
    const std::shared_ptr<DocumentsWriterDeleteQueue> queue;
    const std::shared_ptr<AtomicInteger> index;
    std::deque<std::optional<int>> const ids;
    const std::shared_ptr<DeleteSlice> slice;
    const std::shared_ptr<BufferedUpdates> deletes;
    const std::shared_ptr<CountDownLatch> latch;

  protected:
    UpdateThread(std::shared_ptr<DocumentsWriterDeleteQueue> queue,
                 std::shared_ptr<AtomicInteger> index,
                 std::deque<Integer> &ids,
                 std::shared_ptr<CountDownLatch> latch);

  public:
    void run() override;

  protected:
    std::shared_ptr<UpdateThread> shared_from_this()
    {
      return std::static_pointer_cast<UpdateThread>(Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestDocumentsWriterDeleteQueue> shared_from_this()
  {
    return std::static_pointer_cast<TestDocumentsWriterDeleteQueue>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
