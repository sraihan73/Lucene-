#pragma once
#include "stringhelper.h"
#include <memory>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Collector.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"

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

class TestMultiCollector : public LuceneTestCase
{
  GET_CLASS_NAME(TestMultiCollector)

private:
  class TerminateAfterCollector : public FilterCollector
  {
    GET_CLASS_NAME(TerminateAfterCollector)

  private:
    int count = 0;
    const int terminateAfter;

  public:
    TerminateAfterCollector(std::shared_ptr<Collector> in_, int terminateAfter);

    std::shared_ptr<LeafCollector> getLeafCollector(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class FilterLeafCollectorAnonymousInnerClass : public FilterLeafCollector
    {
      GET_CLASS_NAME(FilterLeafCollectorAnonymousInnerClass)
    private:
      std::shared_ptr<TerminateAfterCollector> outerInstance;

    public:
      FilterLeafCollectorAnonymousInnerClass(
          std::shared_ptr<TerminateAfterCollector> outerInstance);

      void collect(int doc)  override;

    protected:
      std::shared_ptr<FilterLeafCollectorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterLeafCollectorAnonymousInnerClass>(
            FilterLeafCollector::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<TerminateAfterCollector> shared_from_this()
    {
      return std::static_pointer_cast<TerminateAfterCollector>(
          FilterCollector::shared_from_this());
    }
  };

private:
  class SetScorerCollector : public FilterCollector
  {
    GET_CLASS_NAME(SetScorerCollector)

  private:
    const std::shared_ptr<AtomicBoolean> setScorerCalled;

  public:
    SetScorerCollector(std::shared_ptr<Collector> in_,
                       std::shared_ptr<AtomicBoolean> setScorerCalled);

    std::shared_ptr<LeafCollector> getLeafCollector(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class FilterLeafCollectorAnonymousInnerClass : public FilterLeafCollector
    {
      GET_CLASS_NAME(FilterLeafCollectorAnonymousInnerClass)
    private:
      std::shared_ptr<SetScorerCollector> outerInstance;

    public:
      FilterLeafCollectorAnonymousInnerClass(
          std::shared_ptr<SetScorerCollector> outerInstance,
          std::shared_ptr<org::apache::lucene::search::LeafCollector>
              getLeafCollector);

      void
      setScorer(std::shared_ptr<Scorer> scorer)  override;

    protected:
      std::shared_ptr<FilterLeafCollectorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterLeafCollectorAnonymousInnerClass>(
            FilterLeafCollector::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<SetScorerCollector> shared_from_this()
    {
      return std::static_pointer_cast<SetScorerCollector>(
          FilterCollector::shared_from_this());
    }
  };

public:
  virtual void testCollectionTerminatedExceptionHandling() ;

  virtual void testSetScorerAfterCollectionTerminated() ;

protected:
  std::shared_ptr<TestMultiCollector> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiCollector>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
