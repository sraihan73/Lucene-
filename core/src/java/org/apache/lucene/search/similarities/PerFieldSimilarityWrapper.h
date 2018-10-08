#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/FieldInvertState.h"

#include  "core/src/java/org/apache/lucene/search/CollectionStatistics.h"
#include  "core/src/java/org/apache/lucene/search/TermStatistics.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"

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
namespace org::apache::lucene::search::similarities
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using TermStatistics = org::apache::lucene::search::TermStatistics;

/**
 * Provides the ability to use a different {@link Similarity} for different
 * fields. <p> Subclasses should implement {@link #get(std::wstring)} to return an
 * appropriate Similarity (for example, using field-specific parameter values)
 * for the field.
 *
 * @lucene.experimental
 */
class PerFieldSimilarityWrapper : public Similarity
{
  GET_CLASS_NAME(PerFieldSimilarityWrapper)

  /**
   * Sole constructor. (For invocation by subclass
   * constructors, typically implicit.)
   */
public:
  PerFieldSimilarityWrapper();

  int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override final;

  std::shared_ptr<SimWeight>
  computeWeight(float boost,
                std::shared_ptr<CollectionStatistics> collectionStats,
                std::deque<TermStatistics> &termStats) override final;

  std::shared_ptr<SimScorer>
  simScorer(std::shared_ptr<SimWeight> weight,
            std::shared_ptr<LeafReaderContext> context) 
      override final;

  /**
   * Returns a {@link Similarity} for scoring a field.
   */
  virtual std::shared_ptr<Similarity> get(const std::wstring &name) = 0;

public:
  class PerFieldSimWeight : public SimWeight
  {
    GET_CLASS_NAME(PerFieldSimWeight)
  public:
    std::shared_ptr<Similarity> delegate_;
    std::shared_ptr<SimWeight> delegateWeight;

  protected:
    std::shared_ptr<PerFieldSimWeight> shared_from_this()
    {
      return std::static_pointer_cast<PerFieldSimWeight>(
          SimWeight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<PerFieldSimilarityWrapper> shared_from_this()
  {
    return std::static_pointer_cast<PerFieldSimilarityWrapper>(
        Similarity::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
