#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
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
 * Base {@link Collector} implementation that is used to collect all contexts.
 *
 * @lucene.experimental
 */
class SimpleCollector : public std::enable_shared_from_this<SimpleCollector>,
                        public Collector,
                        public LeafCollector
{
  GET_CLASS_NAME(SimpleCollector)

public:
  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

  /** This method is called before collecting <code>context</code>. */
protected:
  virtual void doSetNextReader(
      std::shared_ptr<LeafReaderContext> context) ;

public:
  void setScorer(std::shared_ptr<Scorer> scorer)  override;

  // redeclare methods so that javadocs are inherited on sub-classes

  void collect(int doc) = 0;
  override
};

} // #include  "core/src/java/org/apache/lucene/search/
