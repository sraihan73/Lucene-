#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Collector.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
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

/**
 * A collector that asserts that it is used correctly.
 */
class AssertingCollector : public FilterCollector
{
  GET_CLASS_NAME(AssertingCollector)

private:
  const std::shared_ptr<Random> random;
  int maxDoc = -1;

  /** Wrap the given collector in order to add assertions. */
public:
  static std::shared_ptr<Collector> wrap(std::shared_ptr<Random> random,
                                         std::shared_ptr<Collector> in_);

private:
  AssertingCollector(std::shared_ptr<Random> random,
                     std::shared_ptr<Collector> in_);

public:
  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class AssertingLeafCollectorAnonymousInnerClass
      : public AssertingLeafCollector
  {
    GET_CLASS_NAME(AssertingLeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<AssertingCollector> outerInstance;

    int docBase = 0;

  public:
    AssertingLeafCollectorAnonymousInnerClass(
        std::shared_ptr<AssertingCollector> outerInstance,
        std::shared_ptr<Random> random, int NO_MORE_DOCS, int docBase);

    void collect(int doc)  override;

  protected:
    std::shared_ptr<AssertingLeafCollectorAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          AssertingLeafCollectorAnonymousInnerClass>(
          AssertingLeafCollector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<AssertingCollector> shared_from_this()
  {
    return std::static_pointer_cast<AssertingCollector>(
        FilterCollector::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
