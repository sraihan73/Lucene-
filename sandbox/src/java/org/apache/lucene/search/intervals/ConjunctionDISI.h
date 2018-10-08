#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

#include  "core/src/java/org/apache/lucene/search/ConjunctionDISI.h"

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
 * A conjunction of DocIdSetIterators.
 * This iterates over the doc ids that are present in each given
 * DocIdSetIterator.
 *
 * @lucene.internal
 */
class ConjunctionDISI final : public DocIdSetIterator
{
  GET_CLASS_NAME(ConjunctionDISI)

  /**
   * Create a conjunction over the provided DocIdSetIterators.
   */
public:
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public static
  // org.apache.lucene.search.DocIdSetIterator
  // intersectIterators(java.util.List<? extends
  // org.apache.lucene.search.DocIdSetIterator> iterators)
  static std::shared_ptr<DocIdSetIterator>
  intersectIterators(std::deque<T1> iterators);

private:
  static void
  addIterator(std::shared_ptr<DocIdSetIterator> disi,
              std::deque<std::shared_ptr<DocIdSetIterator>> &allIterators);

public:
  const std::shared_ptr<DocIdSetIterator> lead1, lead2;
  std::deque<std::shared_ptr<DocIdSetIterator>> const others;

private:
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: private ConjunctionDISI(java.util.List<? extends
  // org.apache.lucene.search.DocIdSetIterator> iterators)
  ConjunctionDISI(std::deque<T1> iterators);

  int doNext(int doc) ;

public:
  int advance(int target)  override;

  int docID() override;

  int nextDoc()  override;

  int64_t cost() override;

protected:
  std::shared_ptr<ConjunctionDISI> shared_from_this()
  {
    return std::static_pointer_cast<ConjunctionDISI>(
        org.apache.lucene.search.DocIdSetIterator::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/intervals/
