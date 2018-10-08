#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriter;
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestStressDeletes : public LuceneTestCase
{
  GET_CLASS_NAME(TestStressDeletes)

  /** Make sure that order of adds/deletes across threads is respected as
   *  long as each ID is only changed by one thread at a time. */
public:
  virtual void test() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestStressDeletes> outerInstance;

    int numIDs = 0;
    std::deque<std::any> locks;
    std::shared_ptr<org::apache::lucene::index::IndexWriter> w;
    int iters = 0;
    std::unordered_map<int, bool> exists;
    std::shared_ptr<CountDownLatch> startingGun;
    int deleteMode = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestStressDeletes> outerInstance, int numIDs,
        std::deque<std::any> &locks,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> w, int iters,
        std::unordered_map<int, bool> &exists,
        std::shared_ptr<CountDownLatch> startingGun, int deleteMode);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestStressDeletes> shared_from_this()
  {
    return std::static_pointer_cast<TestStressDeletes>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
