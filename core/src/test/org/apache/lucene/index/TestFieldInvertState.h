#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/FieldInvertState.h"

#include  "core/src/java/org/apache/lucene/search/CollectionStatistics.h"
#include  "core/src/java/org/apache/lucene/search/TermStatistics.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimWeight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"

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

using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestFieldInvertState : public LuceneTestCase
{
  GET_CLASS_NAME(TestFieldInvertState)
  /**
   * Similarity holds onto the FieldInvertState for subsequent verification.
   */
private:
  class NeverForgetsSimilarity : public Similarity
  {
    GET_CLASS_NAME(NeverForgetsSimilarity)
  public:
    std::shared_ptr<FieldInvertState> lastState;

  private:
    static const std::shared_ptr<NeverForgetsSimilarity> INSTANCE;

    NeverForgetsSimilarity();

  public:
    int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override;

    std::shared_ptr<Similarity::SimWeight>
    computeWeight(float boost,
                  std::shared_ptr<CollectionStatistics> collectionStats,
                  std::deque<TermStatistics> &termStats) override;

    std::shared_ptr<Similarity::SimScorer> simScorer(
        std::shared_ptr<Similarity::SimWeight> weight,
        std::shared_ptr<LeafReaderContext> context)  override;

  protected:
    std::shared_ptr<NeverForgetsSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<NeverForgetsSimilarity>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

public:
  virtual void testBasic() ;

  virtual void testRandom() ;

protected:
  std::shared_ptr<TestFieldInvertState> shared_from_this()
  {
    return std::static_pointer_cast<TestFieldInvertState>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
