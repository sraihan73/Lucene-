#pragma once
#include "stringhelper.h"
#include <memory>

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

/**
 * A {@link Collector} implementation which wraps another
 * {@link Collector} and makes sure only documents with
 * scores &gt; 0 are collected.
 */
class PositiveScoresOnlyCollector : public FilterCollector
{
  GET_CLASS_NAME(PositiveScoresOnlyCollector)

public:
  PositiveScoresOnlyCollector(std::shared_ptr<Collector> in_);

  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class FilterLeafCollectorAnonymousInnerClass : public FilterLeafCollector
  {
    GET_CLASS_NAME(FilterLeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<PositiveScoresOnlyCollector> outerInstance;

  public:
    FilterLeafCollectorAnonymousInnerClass(
        std::shared_ptr<PositiveScoresOnlyCollector> outerInstance,
        std::shared_ptr<org::apache::lucene::search::LeafCollector>
            getLeafCollector);

  private:
    std::shared_ptr<Scorer> scorer;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer)  override;

    void collect(int doc)  override;

  protected:
    std::shared_ptr<FilterLeafCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterLeafCollectorAnonymousInnerClass>(
          FilterLeafCollector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<PositiveScoresOnlyCollector> shared_from_this()
  {
    return std::static_pointer_cast<PositiveScoresOnlyCollector>(
        FilterCollector::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
