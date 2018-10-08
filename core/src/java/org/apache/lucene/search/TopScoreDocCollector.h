#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"

#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"

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

/**
 * A {@link Collector} implementation that collects the top-scoring hits,
 * returning them as a {@link TopDocs}. This is used by {@link IndexSearcher} to
 * implement {@link TopDocs}-based search. Hits are sorted by score descending
 * and then (when the scores are tied) docID ascending. When you create an
 * instance of this collector you should know in advance whether documents are
 * going to be collected in doc Id order or not.
 *
 * <p><b>NOTE</b>: The values {@link Float#NaN} and
 * {@link Float#NEGATIVE_INFINITY} are not valid scores.  This
 * collector will not properly collect hits with such
 * scores.
 */
class TopScoreDocCollector : public TopDocsCollector<std::shared_ptr<ScoreDoc>>
{
  GET_CLASS_NAME(TopScoreDocCollector)

public:
  class ScorerLeafCollector
      : public std::enable_shared_from_this<ScorerLeafCollector>,
        public LeafCollector
  {
    GET_CLASS_NAME(ScorerLeafCollector)

  public:
    std::shared_ptr<Scorer> scorer;

    void setScorer(std::shared_ptr<Scorer> scorer)  override;
  };

private:
  class SimpleTopScoreDocCollector;

private:
  class PagingTopScoreDocCollector;

  /**
   * Creates a new {@link TopScoreDocCollector} given the number of hits to
   * collect and whether documents are scored in order by the input
   * {@link Scorer} to {@link LeafCollector#setScorer(Scorer)}.
   *
   * <p><b>NOTE</b>: The instances returned by this method
   * pre-allocate a full array of length
   * <code>numHits</code>, and fill the array with sentinel
   * objects.
   */
public:
  static std::shared_ptr<TopScoreDocCollector> create(int numHits);

  /**
   * Creates a new {@link TopScoreDocCollector} given the number of hits to
   * collect, the bottom of the previous page, and whether documents are scored
   * in order by the input
   * {@link Scorer} to {@link LeafCollector#setScorer(Scorer)}.
   *
   * <p><b>NOTE</b>: The instances returned by this method
   * pre-allocate a full array of length
   * <code>numHits</code>, and fill the array with sentinel
   * objects.
   */
  static std::shared_ptr<TopScoreDocCollector>
  create(int numHits, std::shared_ptr<ScoreDoc> after);

  std::shared_ptr<ScoreDoc> pqTop;

  // prevents instantiation
  TopScoreDocCollector(int numHits);

protected:
  std::shared_ptr<TopDocs>
  newTopDocs(std::deque<std::shared_ptr<ScoreDoc>> &results,
             int start) override;

public:
  bool needsScores() override;

protected:
  std::shared_ptr<TopScoreDocCollector> shared_from_this()
  {
    return std::static_pointer_cast<TopScoreDocCollector>(
        TopDocsCollector<ScoreDoc>::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
class TopScoreDocCollector::SimpleTopScoreDocCollector
    : public TopScoreDocCollector
{
  GET_CLASS_NAME(TopScoreDocCollector::SimpleTopScoreDocCollector)

public:
  SimpleTopScoreDocCollector(int numHits);

  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class ScorerLeafCollectorAnonymousInnerClass : public ScorerLeafCollector
  {
    GET_CLASS_NAME(ScorerLeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleTopScoreDocCollector> outerInstance;

    int docBase = 0;

  public:
    ScorerLeafCollectorAnonymousInnerClass(
        std::shared_ptr<SimpleTopScoreDocCollector> outerInstance, int docBase);

    void collect(int doc)  override;

  protected:
    std::shared_ptr<ScorerLeafCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ScorerLeafCollectorAnonymousInnerClass>(
          ScorerLeafCollector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SimpleTopScoreDocCollector> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTopScoreDocCollector>(
        TopScoreDocCollector::shared_from_this());
  }
};
class TopScoreDocCollector::PagingTopScoreDocCollector
    : public TopScoreDocCollector
{
  GET_CLASS_NAME(TopScoreDocCollector::PagingTopScoreDocCollector)

private:
  const std::shared_ptr<ScoreDoc> after;
  int collectedHits = 0;

public:
  PagingTopScoreDocCollector(int numHits, std::shared_ptr<ScoreDoc> after);

protected:
  int topDocsSize() override;

  std::shared_ptr<TopDocs>
  newTopDocs(std::deque<std::shared_ptr<ScoreDoc>> &results,
             int start) override;

public:
  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class ScorerLeafCollectorAnonymousInnerClass : public ScorerLeafCollector
  {
    GET_CLASS_NAME(ScorerLeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<PagingTopScoreDocCollector> outerInstance;

    int docBase = 0;
    int afterDoc = 0;

  public:
    ScorerLeafCollectorAnonymousInnerClass(
        std::shared_ptr<PagingTopScoreDocCollector> outerInstance, int docBase,
        int afterDoc);

    void collect(int doc)  override;

  protected:
    std::shared_ptr<ScorerLeafCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ScorerLeafCollectorAnonymousInnerClass>(
          ScorerLeafCollector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<PagingTopScoreDocCollector> shared_from_this()
  {
    return std::static_pointer_cast<PagingTopScoreDocCollector>(
        TopScoreDocCollector::shared_from_this());
  }
};
