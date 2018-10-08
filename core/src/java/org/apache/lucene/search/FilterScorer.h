#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Scorer.h"

#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"

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

/**
 * A {@code FilterScorer} contains another {@code Scorer}, which it
 * uses as its basic source of data, possibly transforming the data along the
 * way or providing additional functionality. The class
 * {@code FilterScorer} itself simply implements all abstract methods
 * of {@code Scorer} with versions that pass all requests to the
 * contained scorer. Subclasses of {@code FilterScorer} may
 * further override some of these methods and may also provide additional
 * methods and fields.
 */
class FilterScorer : public Scorer
{
  GET_CLASS_NAME(FilterScorer)
protected:
  const std::shared_ptr<Scorer> in_;

  /**
   * Create a new FilterScorer
   * @param in the {@link Scorer} to wrap
   */
public:
  FilterScorer(std::shared_ptr<Scorer> in_);

  /**
   * Create a new FilterScorer with a specific weight
   * @param in the {@link Scorer} to wrap
   * @param weight a {@link Weight}
   */
  FilterScorer(std::shared_ptr<Scorer> in_, std::shared_ptr<Weight> weight);

  float score()  override;

  int docID() override final;

  std::shared_ptr<DocIdSetIterator> iterator() override final;

  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override final;

protected:
  std::shared_ptr<FilterScorer> shared_from_this()
  {
    return std::static_pointer_cast<FilterScorer>(Scorer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
