#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"

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
 * Re-scores the topN results ({@link TopDocs}) from an original
 * query.  See {@link QueryRescorer} for an actual
 * implementation.  Typically, you run a low-cost
 * first-pass query across the entire index, collecting the
 * top few hundred hits perhaps, and then use this class to
 * mix in a more costly second pass scoring.
 *
 * <p>See {@link
 * QueryRescorer#rescore(IndexSearcher,TopDocs,Query,double,int)}
 * for a simple static method to call to rescore using a 2nd
 * pass {@link Query}.
 *
 * @lucene.experimental
 */

class Rescorer : public std::enable_shared_from_this<Rescorer>
{
  GET_CLASS_NAME(Rescorer)

  /**
   * Rescore an initial first-pass {@link TopDocs}.
   *
   * @param searcher {@link IndexSearcher} used to produce the
   *   first pass topDocs
   * @param firstPassTopDocs Hits from the first pass
   *   search.  It's very important that these hits were
   *   produced by the provided searcher; otherwise the doc
   *   IDs will not match!
   * @param topN How many re-scored hits to return
   */
public:
  virtual std::shared_ptr<TopDocs>
  rescore(std::shared_ptr<IndexSearcher> searcher,
          std::shared_ptr<TopDocs> firstPassTopDocs, int topN) = 0;

  /**
   * Explains how the score for the specified document was
   * computed.
   */
  virtual std::shared_ptr<Explanation>
  explain(std::shared_ptr<IndexSearcher> searcher,
          std::shared_ptr<Explanation> firstPassExplanation, int docID) = 0;
};

} // #include  "core/src/java/org/apache/lucene/search/
