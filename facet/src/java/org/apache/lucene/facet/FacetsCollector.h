#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/facet/MatchingDocs.h"
#include  "core/src/java/org/apache/lucene/util/DocIdSetBuilder.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSet.h"
#include  "core/src/java/org/apache/lucene/search/Collector.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
#include  "core/src/java/org/apache/lucene/search/Sort.h"
#include  "core/src/java/org/apache/lucene/search/TopFieldDocs.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"

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
namespace org::apache::lucene::facet
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Collector = org::apache::lucene::search::Collector;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using Sort = org::apache::lucene::search::Sort;
using TopDocs = org::apache::lucene::search::TopDocs;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;

/** Collects hits for subsequent faceting.  Once you've run
 *  a search and collect hits into this, instantiate one of
 *  the {@link Facets} subclasses to do the facet
 *  counting.  Use the {@code search} utility methods to
GET_CLASS_NAME(es)
 *  perform an "ordinary" search but also collect into a
 *  {@link Collector}. */
// redundant 'implements Collector' to workaround javadocs bugs
class FacetsCollector : public SimpleCollector, public Collector
{
  GET_CLASS_NAME(FacetsCollector)

private:
  std::shared_ptr<LeafReaderContext> context;
  std::shared_ptr<Scorer> scorer;
  int totalHits = 0;
  std::deque<float> scores;
  const bool keepScores;
  const std::deque<std::shared_ptr<MatchingDocs>> matchingDocs =
      std::deque<std::shared_ptr<MatchingDocs>>();
  std::shared_ptr<DocIdSetBuilder> docsBuilder;

  /**
   * Holds the documents that were matched in the {@link
   * org.apache.lucene.index.LeafReaderContext}. If scores were required, then
   * {@code scores} is not null.
   */
public:
  class MatchingDocs final : public std::enable_shared_from_this<MatchingDocs>
  {
    GET_CLASS_NAME(MatchingDocs)

    /** Context for this segment. */
  public:
    const std::shared_ptr<LeafReaderContext> context;

    /** Which documents were seen. */
    const std::shared_ptr<DocIdSet> bits;

    /** Non-sparse scores array. */
    std::deque<float> const scores;

    /** Total number of hits */
    const int totalHits;

    /** Sole constructor. */
    MatchingDocs(std::shared_ptr<LeafReaderContext> context,
                 std::shared_ptr<DocIdSet> bits, int totalHits,
                 std::deque<float> &scores);
  };

  /** Default constructor */
public:
  FacetsCollector();

  /** Create this; if {@code keepScores} is true then a
   *  float[] is allocated to hold score of all hits. */
  FacetsCollector(bool keepScores);

  /** True if scores were saved. */
  bool getKeepScores();

  /**
   * Returns the documents matched by the query, one {@link MatchingDocs} per
   * visited segment.
   */
  virtual std::deque<std::shared_ptr<MatchingDocs>> getMatchingDocs();

  void collect(int doc)  override final;

  bool needsScores() override;

  void
  setScorer(std::shared_ptr<Scorer> scorer)  override final;

protected:
  void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
      IOException) override;

  /** Utility method, to search and also collect all hits
   *  into the provided {@link Collector}. */
public:
  static std::shared_ptr<TopDocs>
  search(std::shared_ptr<IndexSearcher> searcher, std::shared_ptr<Query> q,
         int n, std::shared_ptr<Collector> fc) ;

  /** Utility method, to search and also collect all hits
   *  into the provided {@link Collector}. */
  static std::shared_ptr<TopFieldDocs>
  search(std::shared_ptr<IndexSearcher> searcher, std::shared_ptr<Query> q,
         int n, std::shared_ptr<Sort> sort,
         std::shared_ptr<Collector> fc) ;

  /** Utility method, to search and also collect all hits
   *  into the provided {@link Collector}. */
  static std::shared_ptr<TopFieldDocs>
  search(std::shared_ptr<IndexSearcher> searcher, std::shared_ptr<Query> q,
         int n, std::shared_ptr<Sort> sort, bool doDocScores, bool doMaxScore,
         std::shared_ptr<Collector> fc) ;

  /** Utility method, to search and also collect all hits
   *  into the provided {@link Collector}. */
  static std::shared_ptr<TopDocs>
  searchAfter(std::shared_ptr<IndexSearcher> searcher,
              std::shared_ptr<ScoreDoc> after, std::shared_ptr<Query> q, int n,
              std::shared_ptr<Collector> fc) ;

  /** Utility method, to search and also collect all hits
   *  into the provided {@link Collector}. */
  static std::shared_ptr<TopDocs>
  searchAfter(std::shared_ptr<IndexSearcher> searcher,
              std::shared_ptr<ScoreDoc> after, std::shared_ptr<Query> q, int n,
              std::shared_ptr<Sort> sort,
              std::shared_ptr<Collector> fc) ;

  /** Utility method, to search and also collect all hits
   *  into the provided {@link Collector}. */
  static std::shared_ptr<TopDocs>
  searchAfter(std::shared_ptr<IndexSearcher> searcher,
              std::shared_ptr<ScoreDoc> after, std::shared_ptr<Query> q, int n,
              std::shared_ptr<Sort> sort, bool doDocScores, bool doMaxScore,
              std::shared_ptr<Collector> fc) ;

private:
  static std::shared_ptr<TopDocs>
  doSearch(std::shared_ptr<IndexSearcher> searcher,
           std::shared_ptr<ScoreDoc> after, std::shared_ptr<Query> q, int n,
           std::shared_ptr<Sort> sort, bool doDocScores, bool doMaxScore,
           std::shared_ptr<Collector> fc) ;

protected:
  std::shared_ptr<FacetsCollector> shared_from_this()
  {
    return std::static_pointer_cast<FacetsCollector>(
        org.apache.lucene.search.SimpleCollector::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
