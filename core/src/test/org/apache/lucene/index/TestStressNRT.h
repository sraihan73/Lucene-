#pragma once
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class DirectoryReader;
}

namespace org::apache::lucene::document
{
class FieldType;
}
namespace org::apache::lucene::index
{
class RandomIndexWriter;
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

class TestStressNRT : public LuceneTestCase
{
  GET_CLASS_NAME(TestStressNRT)
public:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile DirectoryReader reader;
  std::shared_ptr<DirectoryReader> reader;

  const std::shared_ptr<ConcurrentHashMap<int, int64_t>> model =
      std::make_shared<ConcurrentHashMap<int, int64_t>>();
  std::unordered_map<int, int64_t> committedModel =
      std::unordered_map<int, int64_t>();
  int64_t snapshotCount = 0;
  int64_t committedModelClock = 0;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile int lastId;
  int lastId = 0;
  const std::wstring field = L"val_l";
  std::deque<std::any> syncArr;

private:
  void initModel(int ndocs);

public:
  virtual void test() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestStressNRT> outerInstance;

    int commitPercent = 0;
    int softCommitPercent = 0;
    int deletePercent = 0;
    int deleteByQueryPercent = 0;
    int ndocs = 0;
    int maxConcurrentCommits = 0;
    bool tombstones = false;
    std::shared_ptr<AtomicLong> operations;
    std::shared_ptr<FieldType> storedOnlyType;
    std::shared_ptr<AtomicInteger> numCommitting;
    std::shared_ptr<org::apache::lucene::index::RandomIndexWriter> writer;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestStressNRT> outerInstance,
        std::wstring L"WRITER" + i, int commitPercent, int softCommitPercent,
        int deletePercent, int deleteByQueryPercent, int ndocs,
        int maxConcurrentCommits, bool tombstones,
        std::shared_ptr<AtomicLong> operations,
        std::shared_ptr<FieldType> storedOnlyType,
        std::shared_ptr<AtomicInteger> numCommitting,
        std::shared_ptr<org::apache::lucene::index::RandomIndexWriter> writer);

    std::shared_ptr<Random> rand;

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<TestStressNRT> outerInstance;

    int ndocs = 0;
    bool tombstones = false;
    std::shared_ptr<AtomicLong> operations;

  public:
    ThreadAnonymousInnerClass2(std::shared_ptr<TestStressNRT> outerInstance,
                               std::wstring L"READER" + i, int ndocs,
                               bool tombstones,
                               std::shared_ptr<AtomicLong> operations);

    std::shared_ptr<Random> rand;

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestStressNRT> shared_from_this()
  {
    return std::static_pointer_cast<TestStressNRT>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
