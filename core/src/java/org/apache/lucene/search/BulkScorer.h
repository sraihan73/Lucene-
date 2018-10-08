#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"

#include  "core/src/java/org/apache/lucene/util/Bits.h"

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

using Bits = org::apache::lucene::util::Bits;

/** This class is used to score a range of documents at
 *  once, and is returned by {@link Weight#bulkScorer}.  Only
GET_CLASS_NAME(is)
 *  queries that have a more optimized means of scoring
 *  across a range of documents need to override this.
 *  Otherwise, a default implementation is wrapped around
 *  the {@link Scorer} returned by {@link Weight#scorer}. */

class BulkScorer : public std::enable_shared_from_this<BulkScorer>
{
  GET_CLASS_NAME(BulkScorer)

  /** Scores and collects all matching documents.
   * @param collector The collector to which all matching documents are passed.
   * @param acceptDocs {@link Bits} that represents the allowed documents to
   * match, or
   *                   {@code null} if they are all allowed to match.
   */
public:
  virtual void score(std::shared_ptr<LeafCollector> collector,
                     std::shared_ptr<Bits> acceptDocs) ;

  /**
   * Collects matching documents in a range and return an estimation of the
   * next matching document which is on or after {@code max}.
   * <p>The return value must be:</p><ul>
   *   <li>&gt;= {@code max},</li>
   *   <li>{@link DocIdSetIterator#NO_MORE_DOCS} if there are no more
   * matches,</li> <li>&lt;= the first matching document that is &gt;= {@code
   * max} otherwise.</li>
   * </ul>
   * <p>{@code min} is the minimum document to be considered for matching. All
   * documents strictly before this value must be ignored.</p>
   * <p>Although {@code max} would be a legal return value for this method,
   * higher values might help callers skip more efficiently over non-matching
   * portions of the docID space.</p> <p>For instance, a {@link Scorer}-based
   * implementation could look like below:</p> <pre class="prettyprint"> private
   * final Scorer scorer; // set via constructor
   *
   * public int score(LeafCollector collector, Bits acceptDocs, int min, int
   * max) throws IOException { collector.setScorer(scorer); int doc =
   * scorer.docID(); if (doc &lt; min) { doc = scorer.advance(min);
   *   }
   *   while (doc &lt; max) {
   *     if (acceptDocs == null || acceptDocs.get(doc)) {
   *       collector.collect(doc);
   *     }
   *     doc = scorer.nextDoc();
   *   }
   *   return doc;
   * }
   * </pre>
   *
   * @param  collector The collector to which all matching documents are passed.
   * @param acceptDocs {@link Bits} that represents the allowed documents to
   * match, or
   *                   {@code null} if they are all allowed to match.
   * @param  min Score starting at, including, this document
   * @param  max Score up to, but not including, this doc
   * @return an under-estimation of the next matching doc after max
   */
  virtual int score(std::shared_ptr<LeafCollector> collector,
                    std::shared_ptr<Bits> acceptDocs, int min, int max) = 0;

  /**
   * Same as {@link DocIdSetIterator#cost()} for bulk scorers.
   */
  virtual int64_t cost() = 0;
};

} // #include  "core/src/java/org/apache/lucene/search/
