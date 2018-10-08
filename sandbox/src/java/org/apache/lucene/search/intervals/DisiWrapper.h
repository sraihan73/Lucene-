#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

#include  "core/src/java/org/apache/lucene/search/intervals/IntervalIterator.h"
#include  "core/src/java/org/apache/lucene/search/DisiWrapper.h"

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

class DisiWrapper : public std::enable_shared_from_this<DisiWrapper>
{
  GET_CLASS_NAME(DisiWrapper)

public:
  const std::shared_ptr<DocIdSetIterator> iterator;
  const std::shared_ptr<IntervalIterator> intervals;
  const int64_t cost;
  const float matchCost; // the match cost for two-phase iterators, 0 otherwise
  int doc = 0;           // the current doc, used for comparison
  std::shared_ptr<DisiWrapper>
      next; // reference to a next element, see #topList

  // An approximation of the iterator, or the iterator itself if it does not
  // support two-phase iteration
  const std::shared_ptr<DocIdSetIterator> approximation;

  DisiWrapper(std::shared_ptr<IntervalIterator> iterator);
};

} // #include  "core/src/java/org/apache/lucene/search/intervals/
