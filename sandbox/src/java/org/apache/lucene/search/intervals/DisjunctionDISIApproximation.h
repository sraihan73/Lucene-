#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DisiPriorityQueue;
}

namespace org::apache::lucene::search::intervals
{
class DisiPriorityQueue;
}
namespace org::apache::lucene::search
{
class DisjunctionDISIApproximation;
}

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
namespace org::apache::lucene::search::intervals
{

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

/**
 * A {@link DocIdSetIterator} which is a disjunction of the approximations of
 * the provided iterators.
 * @lucene.internal
 */
class DisjunctionDISIApproximation : public DocIdSetIterator
{
  GET_CLASS_NAME(DisjunctionDISIApproximation)

public:
  const std::shared_ptr<DisiPriorityQueue> subIterators;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t cost_;

  DisjunctionDISIApproximation(std::shared_ptr<DisiPriorityQueue> subIterators);

  int64_t cost() override;

  int docID() override;

  int nextDoc()  override;

  int advance(int target)  override;

protected:
  std::shared_ptr<DisjunctionDISIApproximation> shared_from_this()
  {
    return std::static_pointer_cast<DisjunctionDISIApproximation>(
        org.apache.lucene.search.DocIdSetIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::intervals