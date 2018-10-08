#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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

class TestCachingCollector : public LuceneTestCase
{
  GET_CLASS_NAME(TestCachingCollector)

private:
  static constexpr double ONE_BYTE = 1.0 / (1024 * 1024); // 1 byte out of MB

private:
  class MockScorer : public Scorer
  {
    GET_CLASS_NAME(MockScorer)

  private:
    MockScorer();

  public:
    float score()  override;
    int docID() override;
    std::shared_ptr<DocIdSetIterator> iterator() override;

  protected:
    std::shared_ptr<MockScorer> shared_from_this()
    {
      return std::static_pointer_cast<MockScorer>(Scorer::shared_from_this());
    }
  };

private:
  class NoOpCollector : public SimpleCollector
  {
    GET_CLASS_NAME(NoOpCollector)

  public:
    void collect(int doc)  override;
    bool needsScores() override;

  protected:
    std::shared_ptr<NoOpCollector> shared_from_this()
    {
      return std::static_pointer_cast<NoOpCollector>(
          SimpleCollector::shared_from_this());
    }
  };

public:
  virtual void testBasic() ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestCachingCollector> outerInstance;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<TestCachingCollector> outerInstance);

    int prevDocID = 0;

    void collect(int doc) override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          SimpleCollector::shared_from_this());
    }
  };

public:
  virtual void testIllegalStateOnReplay() ;

  virtual void testCachedArraysAllocation() ;

  virtual void testNoWrappedCollector() ;

protected:
  std::shared_ptr<TestCachingCollector> shared_from_this()
  {
    return std::static_pointer_cast<TestCachingCollector>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
