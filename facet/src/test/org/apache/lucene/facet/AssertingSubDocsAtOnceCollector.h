#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::facet
{

using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;

/** Verifies in collect() that all child subScorers are on
 *  the collected doc. */
class AssertingSubDocsAtOnceCollector : public SimpleCollector
{
  GET_CLASS_NAME(AssertingSubDocsAtOnceCollector)

  // TODO: allow wrapping another Collector

public:
  std::deque<std::shared_ptr<Scorer>> allScorers;

  void setScorer(std::shared_ptr<Scorer> s)  override;

  void collect(int docID) override;

  bool needsScores() override;

protected:
  std::shared_ptr<AssertingSubDocsAtOnceCollector> shared_from_this()
  {
    return std::static_pointer_cast<AssertingSubDocsAtOnceCollector>(
        org.apache.lucene.search.SimpleCollector::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
