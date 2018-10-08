#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"

#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
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
namespace org::apache::lucene::search::join
{

using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;

class BaseGlobalOrdinalScorer : public Scorer
{
  GET_CLASS_NAME(BaseGlobalOrdinalScorer)

public:
  const std::shared_ptr<SortedDocValues> values;
  const std::shared_ptr<DocIdSetIterator> approximation;

  // C++ NOTE: Fields cannot have the same name as methods:
  float score_ = 0;

  BaseGlobalOrdinalScorer(
      std::shared_ptr<Weight> weight, std::shared_ptr<SortedDocValues> values,
      std::shared_ptr<DocIdSetIterator> approximationScorer);

  float score()  override;

  int docID() override;

  std::shared_ptr<DocIdSetIterator> iterator() override;

  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

protected:
  virtual std::shared_ptr<TwoPhaseIterator>
  createTwoPhaseIterator(std::shared_ptr<DocIdSetIterator> approximation) = 0;

protected:
  std::shared_ptr<BaseGlobalOrdinalScorer> shared_from_this()
  {
    return std::static_pointer_cast<BaseGlobalOrdinalScorer>(
        org.apache.lucene.search.Scorer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/join/
