#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/Collector.h"
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"

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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class MultiCollectorTest : public LuceneTestCase
{
  GET_CLASS_NAME(MultiCollectorTest)

private:
  class DummyCollector : public SimpleCollector
  {
    GET_CLASS_NAME(DummyCollector)

  public:
    bool collectCalled = false;
    bool setNextReaderCalled = false;
    bool setScorerCalled = false;

    void collect(int doc)  override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer)  override;

    bool needsScores() override;

  protected:
    std::shared_ptr<DummyCollector> shared_from_this()
    {
      return std::static_pointer_cast<DummyCollector>(
          SimpleCollector::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testNullCollectors() throws Exception
  virtual void testNullCollectors() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSingleCollector() throws Exception
  virtual void testSingleCollector() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCollector() throws Exception
  virtual void testCollector() ;

private:
  static std::shared_ptr<Collector> collector(bool needsScores,
                                              std::type_info expectedScorer);

private:
  class CollectorAnonymousInnerClass
      : public std::enable_shared_from_this<CollectorAnonymousInnerClass>,
        public Collector
  {
    GET_CLASS_NAME(CollectorAnonymousInnerClass)
  private:
    bool needsScores = false;
    std::type_info expectedScorer;

  public:
    CollectorAnonymousInnerClass(bool needsScores,
                                 std::type_info expectedScorer);

    std::shared_ptr<LeafCollector> getLeafCollector(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class LeafCollectorAnonymousInnerClass
        : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass>,
          public LeafCollector
    {
      GET_CLASS_NAME(LeafCollectorAnonymousInnerClass)
    private:
      std::shared_ptr<CollectorAnonymousInnerClass> outerInstance;

    public:
      LeafCollectorAnonymousInnerClass(
          std::shared_ptr<CollectorAnonymousInnerClass> outerInstance);

      void
      setScorer(std::shared_ptr<Scorer> scorer)  override;

      void collect(int doc)  override;
    };

  public:
    bool needsScores() override;
  };

public:
  virtual void testCacheScoresIfNecessary() ;

protected:
  std::shared_ptr<MultiCollectorTest> shared_from_this()
  {
    return std::static_pointer_cast<MultiCollectorTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
