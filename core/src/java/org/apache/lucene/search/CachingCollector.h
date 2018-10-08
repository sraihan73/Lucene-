#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/NoScoreCachingLeafCollector.h"
#include  "core/src/java/org/apache/lucene/search/Collector.h"
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"

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
 * Caches all docs, and optionally also scores, coming from
 * a search, and is then able to replay them to another
 * collector.  You specify the max RAM this class may use.
 * Once the collection is done, call {@link #isCached}. If
GET_CLASS_NAME(may)
 * this returns true, you can use {@link #replay(Collector)}
 * against a new collector.  If it returns false, this means
 * too much RAM was required and you must instead re-run the
 * original search.
 *
 * <p><b>NOTE</b>: this class consumes 4 (or 8 bytes, if
 * scoring is cached) per collected document.  If the result
 * set is large this can easily be a very substantial amount
 * of RAM!
 *
 * <p>See the Lucene <tt>modules/grouping</tt> module for more
 * details including a full code example.</p>
 *
 * @lucene.experimental
 */
class CachingCollector : public FilterCollector
{
  GET_CLASS_NAME(CachingCollector)

private:
  static constexpr int INITIAL_ARRAY_SIZE = 128;

private:
  class CachedScorer final : public Scorer
  {
    GET_CLASS_NAME(CachedScorer)

    // NOTE: these members are package-private b/c that way accessing them from
    // the outer class does not incur access check by the JVM. The same
    // situation would be if they were defined in the outer class as private
    // members.
  public:
    int doc = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    float score_ = 0;

  private:
    CachedScorer();

  public:
    std::shared_ptr<DocIdSetIterator> iterator() override;

    float score() override final;
    int docID() override;

  protected:
    std::shared_ptr<CachedScorer> shared_from_this()
    {
      return std::static_pointer_cast<CachedScorer>(Scorer::shared_from_this());
    }
  };

private:
  class NoScoreCachingCollector;

private:
  class ScoreCachingCollector : public NoScoreCachingCollector
  {
    GET_CLASS_NAME(ScoreCachingCollector)

  public:
    std::deque<std::deque<float>> scores;

    ScoreCachingCollector(std::shared_ptr<Collector> in_, int maxDocsToCache);

  protected:
    std::shared_ptr<NoScoreCachingLeafCollector>
    wrap(std::shared_ptr<LeafCollector> in_, int maxDocsToCache) override;

    void postCollect(
        std::shared_ptr<NoScoreCachingLeafCollector> collector) override;

    /** Ensure the scores are collected so they can be replayed, even if the
     * wrapped collector doesn't need them. */
  public:
    bool needsScores() override;

  protected:
    void collect(std::shared_ptr<LeafCollector> collector,
                 int i)  override;

  protected:
    std::shared_ptr<ScoreCachingCollector> shared_from_this()
    {
      return std::static_pointer_cast<ScoreCachingCollector>(
          NoScoreCachingCollector::shared_from_this());
    }
  };

private:
  class NoScoreCachingLeafCollector : public FilterLeafCollector
  {
    GET_CLASS_NAME(NoScoreCachingLeafCollector)
  private:
    std::shared_ptr<CachingCollector> outerInstance;

  public:
    const int maxDocsToCache;
    std::deque<int> docs;
    int docCount = 0;

    NoScoreCachingLeafCollector(std::shared_ptr<CachingCollector> outerInstance,
                                std::shared_ptr<LeafCollector> in_,
                                int maxDocsToCache);

  protected:
    virtual void grow(int newLen);

    virtual void invalidate();

    virtual void buffer(int doc) ;

  public:
    void collect(int doc)  override;

    virtual bool hasCache();

    virtual std::deque<int> cachedDocs();

  protected:
    std::shared_ptr<NoScoreCachingLeafCollector> shared_from_this()
    {
      return std::static_pointer_cast<NoScoreCachingLeafCollector>(
          FilterLeafCollector::shared_from_this());
    }
  };

private:
  class ScoreCachingLeafCollector : public NoScoreCachingLeafCollector
  {
    GET_CLASS_NAME(ScoreCachingLeafCollector)
  private:
    std::shared_ptr<CachingCollector> outerInstance;

  public:
    std::shared_ptr<Scorer> scorer;
    std::deque<float> scores;

    ScoreCachingLeafCollector(std::shared_ptr<CachingCollector> outerInstance,
                              std::shared_ptr<LeafCollector> in_,
                              int maxDocsToCache);

    void setScorer(std::shared_ptr<Scorer> scorer)  override;

  protected:
    void grow(int newLen) override;

    void invalidate() override;

    void buffer(int doc)  override;

  public:
    virtual std::deque<float> cachedScores();

  protected:
    std::shared_ptr<ScoreCachingLeafCollector> shared_from_this()
    {
      return std::static_pointer_cast<ScoreCachingLeafCollector>(
          NoScoreCachingLeafCollector::shared_from_this());
    }
  };

  /**
   * Creates a {@link CachingCollector} which does not wrap another collector.
   * The cached documents and scores can later be {@link #replay(Collector)
   * replayed}.
   */
public:
  static std::shared_ptr<CachingCollector> create(bool cacheScores,
                                                  double maxRAMMB);

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  public:
    SimpleCollectorAnonymousInnerClass();

    void collect(int doc) override;
    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          SimpleCollector::shared_from_this());
    }
  };

  /**
   * Create a new {@link CachingCollector} that wraps the given collector and
   * caches documents and scores up to the specified RAM threshold.
   *
   * @param other
   *          the Collector to wrap and delegate calls to.
   * @param cacheScores
   *          whether to cache scores in addition to document IDs. Note that
   *          this increases the RAM consumed per doc
   * @param maxRAMMB
   *          the maximum RAM in MB to consume for caching the documents and
   *          scores. If the collector exceeds the threshold, no documents and
   *          scores are cached.
   */
public:
  static std::shared_ptr<CachingCollector>
  create(std::shared_ptr<Collector> other, bool cacheScores, double maxRAMMB);

  /**
   * Create a new {@link CachingCollector} that wraps the given collector and
   * caches documents and scores up to the specified max docs threshold.
   *
   * @param other
   *          the Collector to wrap and delegate calls to.
   * @param cacheScores
   *          whether to cache scores in addition to document IDs. Note that
   *          this increases the RAM consumed per doc
   * @param maxDocsToCache
   *          the maximum number of documents for caching the documents and
   *          possible the scores. If the collector exceeds the threshold,
   *          no documents and scores are cached.
   */
  static std::shared_ptr<CachingCollector>
  create(std::shared_ptr<Collector> other, bool cacheScores,
         int maxDocsToCache);

private:
  bool cached = false;

  CachingCollector(std::shared_ptr<Collector> in_);

  /**
   * Return true is this collector is able to replay collection.
   */
public:
  bool isCached();

  /**
   * Replays the cached doc IDs (and scores) to the given Collector. If this
   * instance does not cache scores, then Scorer is not set on
   * {@code other.setScorer} as well as scores are not replayed.
   *
   * @throws IllegalStateException
   *           if this collector is not cached (i.e., if the RAM limits were too
   *           low for the number of documents + scores to cache).
   * @throws IllegalArgumentException
   *           if the given Collect's does not support out-of-order collection,
   *           while the collector passed to the ctor does.
   */
  virtual void replay(std::shared_ptr<Collector> other) = 0;

protected:
  std::shared_ptr<CachingCollector> shared_from_this()
  {
    return std::static_pointer_cast<CachingCollector>(
        FilterCollector::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
class CachingCollector::NoScoreCachingCollector : public CachingCollector
{
  GET_CLASS_NAME(CachingCollector::NoScoreCachingCollector)

public:
  std::deque<std::shared_ptr<LeafReaderContext>> contexts;
  std::deque<std::deque<int>> docs;
  int maxDocsToCache = 0;
  std::shared_ptr<NoScoreCachingLeafCollector> lastCollector;

  NoScoreCachingCollector(std::shared_ptr<Collector> in_, int maxDocsToCache);

protected:
  virtual std::shared_ptr<NoScoreCachingLeafCollector>
  wrap(std::shared_ptr<LeafCollector> in_, int maxDocsToCache);

  // note: do *not* override needScore to say false. Just because we aren't
  // caching the score doesn't mean the
  //   wrapped collector doesn't need it to do its job.

public:
  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

protected:
  virtual void invalidate();

  virtual void
  postCollect(std::shared_ptr<NoScoreCachingLeafCollector> collector);

private:
  void postCollection();

protected:
  virtual void collect(std::shared_ptr<LeafCollector> collector,
                       int i) ;

public:
  void replay(std::shared_ptr<Collector> other)  override;

protected:
  std::shared_ptr<NoScoreCachingCollector> shared_from_this()
  {
    return std::static_pointer_cast<NoScoreCachingCollector>(
        CachingCollector::shared_from_this());
  }
};
