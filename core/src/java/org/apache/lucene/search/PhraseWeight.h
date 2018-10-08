#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"

#include  "core/src/java/org/apache/lucene/search/similarities/SimWeight.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/PhraseMatcher.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/Matches.h"

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
using Similarity = org::apache::lucene::search::similarities::Similarity;

class PhraseWeight : public Weight
{
  GET_CLASS_NAME(PhraseWeight)

public:
  const bool needsScores;
  const std::shared_ptr<Similarity::SimWeight> stats;
  const std::shared_ptr<Similarity> similarity;
  const std::wstring field;

protected:
  PhraseWeight(std::shared_ptr<Query> query, const std::wstring &field,
               std::shared_ptr<IndexSearcher> searcher,
               bool needsScores) ;

  virtual std::shared_ptr<Similarity::SimWeight>
  getStats(std::shared_ptr<IndexSearcher> searcher) = 0;

  virtual std::shared_ptr<PhraseMatcher>
  getPhraseMatcher(std::shared_ptr<LeafReaderContext> context,
                   bool exposeOffsets) = 0;

public:
  std::shared_ptr<Scorer> scorer(
      std::shared_ptr<LeafReaderContext> context)  override;

  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> context,
          int doc)  override;

  std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                   int doc)  override;
       );

     protected:
       std::shared_ptr<PhraseWeight> shared_from_this()
       {
         return std::static_pointer_cast<PhraseWeight>(
             Weight::shared_from_this());
       }
};

public:
bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;
} // namespace org::apache::lucene::search
}
