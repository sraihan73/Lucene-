#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/search/TotalHitCountCollector.h"

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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({ "SimpleText", "Memory", "Direct" }) public
// class TestSearchWithThreads extends org.apache.lucene.util.LuceneTestCase
class TestSearchWithThreads : public LuceneTestCase
{
public:
  int NUM_DOCS = 0;
  static constexpr int NUM_SEARCH_THREADS = 5;
  int RUN_TIME_MSEC = 0;

  void setUp()  override;

  virtual void test() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestSearchWithThreads> outerInstance;

    std::shared_ptr<org::apache::lucene::search::IndexSearcher> s;
    std::shared_ptr<AtomicBoolean> failed;
    std::shared_ptr<AtomicLong> netSearch;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestSearchWithThreads> outerInstance,
        std::shared_ptr<org::apache::lucene::search::IndexSearcher> s,
        std::shared_ptr<AtomicBoolean> failed,
        std::shared_ptr<AtomicLong> netSearch);

    std::shared_ptr<TotalHitCountCollector> col;
    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestSearchWithThreads> shared_from_this()
  {
    return std::static_pointer_cast<TestSearchWithThreads>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
