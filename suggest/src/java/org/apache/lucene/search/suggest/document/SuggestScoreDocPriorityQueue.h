#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/suggest/document/TopSuggestDocs.h"

#include  "core/src/java/org/apache/lucene/search/suggest/document/SuggestScoreDoc.h"

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
namespace org::apache::lucene::search::suggest::document
{

using SuggestScoreDoc = org::apache::lucene::search::suggest::document::
    TopSuggestDocs::SuggestScoreDoc;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Bounded priority queue for {@link SuggestScoreDoc}s.
 * Priority is based on {@link SuggestScoreDoc#score} and tie
 * is broken by {@link SuggestScoreDoc#doc}
 */
class SuggestScoreDocPriorityQueue final
    : public PriorityQueue<std::shared_ptr<SuggestScoreDoc>>
{
  GET_CLASS_NAME(SuggestScoreDocPriorityQueue)
  /**
   * Creates a new priority queue of the specified size.
   */
public:
  SuggestScoreDocPriorityQueue(int size);

protected:
  bool lessThan(std::shared_ptr<SuggestScoreDoc> a,
                std::shared_ptr<SuggestScoreDoc> b) override;

  /**
   * Returns the top N results in descending order.
   */
public:
  std::deque<std::shared_ptr<SuggestScoreDoc>> getResults();

protected:
  std::shared_ptr<SuggestScoreDocPriorityQueue> shared_from_this()
  {
    return std::static_pointer_cast<SuggestScoreDocPriorityQueue>(
        org.apache.lucene.util.PriorityQueue<
            org.apache.lucene.search.suggest.document.TopSuggestDocs
                .SuggestScoreDoc>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/document/
