#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class OrdinalMap;
}

namespace org::apache::lucene::util
{
class LongBitSet;
}
namespace org::apache::lucene::search::join
{
class Scores;
}
namespace org::apache::lucene::search::join
{
class Occurrences;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class LeafCollector;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
}
namespace org::apache::lucene::util
{
class LongValues;
}
namespace org::apache::lucene::search
{
class Scorer;
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
namespace org::apache::lucene::search::join
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using Collector = org::apache::lucene::search::Collector;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using Scorer = org::apache::lucene::search::Scorer;
using LongBitSet = org::apache::lucene::util::LongBitSet;
using LongValues = org::apache::lucene::util::LongValues;

class GlobalOrdinalsWithScoreCollector
    : public std::enable_shared_from_this<GlobalOrdinalsWithScoreCollector>,
      public Collector
{
  GET_CLASS_NAME(GlobalOrdinalsWithScoreCollector)

public:
  const std::wstring field;
  const bool doMinMax;
  const int min;
  const int max;
  const std::shared_ptr<OrdinalMap> ordinalMap;
  const std::shared_ptr<LongBitSet> collectedOrds;

protected:
  const std::shared_ptr<Scores> scores;
  const std::shared_ptr<Occurrences> occurrences;

public:
  GlobalOrdinalsWithScoreCollector(const std::wstring &field,
                                   std::shared_ptr<OrdinalMap> ordinalMap,
                                   int64_t valueCount, ScoreMode scoreMode,
                                   int min, int max);

  virtual bool match(int globalOrd);

  virtual float score(int globalOrdinal);

protected:
  virtual void doScore(int globalOrd, float existingScore, float newScore) = 0;

  virtual float unset() = 0;

public:
  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

  bool needsScores() override;

public:
  class OrdinalMapCollector final
      : public std::enable_shared_from_this<OrdinalMapCollector>,
        public LeafCollector
  {
    GET_CLASS_NAME(OrdinalMapCollector)
  private:
    std::shared_ptr<GlobalOrdinalsWithScoreCollector> outerInstance;

    const std::shared_ptr<SortedDocValues> docTermOrds;
    const std::shared_ptr<LongValues> segmentOrdToGlobalOrdLookup;
    std::shared_ptr<Scorer> scorer;

  public:
    OrdinalMapCollector(
        std::shared_ptr<GlobalOrdinalsWithScoreCollector> outerInstance,
        std::shared_ptr<SortedDocValues> docTermOrds,
        std::shared_ptr<LongValues> segmentOrdToGlobalOrdLookup);

    void collect(int doc)  override;

    void setScorer(std::shared_ptr<Scorer> scorer)  override;
  };

public:
  class SegmentOrdinalCollector final
      : public std::enable_shared_from_this<SegmentOrdinalCollector>,
        public LeafCollector
  {
    GET_CLASS_NAME(SegmentOrdinalCollector)
  private:
    std::shared_ptr<GlobalOrdinalsWithScoreCollector> outerInstance;

    const std::shared_ptr<SortedDocValues> docTermOrds;
    std::shared_ptr<Scorer> scorer;

  public:
    SegmentOrdinalCollector(
        std::shared_ptr<GlobalOrdinalsWithScoreCollector> outerInstance,
        std::shared_ptr<SortedDocValues> docTermOrds);

    void collect(int doc)  override;

    void setScorer(std::shared_ptr<Scorer> scorer)  override;
  };

public:
  class Min;

public:
  class Max;

public:
  class Sum;

public:
  class Avg;

public:
  class NoScore;

  // Because the global ordinal is directly used as a key to a score we should
  // be somewhat smart about allocation the scores array. Most of the times not
  // all docs match so splitting the scores array up in blocks can prevent
  // creation of huge arrays. Also working with smaller arrays is supposed to be
  // more gc friendly
  //
  // At first a hash map_obj implementation would make sense, but in the case that
  // more than half of docs match this becomes more expensive then just using an
  // array.

  // Maybe this should become a method parameter?
public:
  static constexpr int arraySize = 4096;

public:
  class Scores final : public std::enable_shared_from_this<Scores>
  {
    GET_CLASS_NAME(Scores)

  public:
    std::deque<std::deque<float>> const blocks;
    const float unset;

  private:
    Scores(int64_t valueCount, float unset);

  public:
    void setScore(int globalOrdinal, float score);

    float getScore(int globalOrdinal);
  };

public:
  class Occurrences final : public std::enable_shared_from_this<Occurrences>
  {
    GET_CLASS_NAME(Occurrences)

  public:
    std::deque<std::deque<int>> const blocks;

  private:
    Occurrences(int64_t valueCount);

  public:
    void increment(int globalOrdinal);

    int getOccurrence(int globalOrdinal);
  };
};

} // namespace org::apache::lucene::search::join
class GlobalOrdinalsWithScoreCollector::Min final
    : public GlobalOrdinalsWithScoreCollector
{
  GET_CLASS_NAME(GlobalOrdinalsWithScoreCollector::Min)

public:
  Min(const std::wstring &field, std::shared_ptr<OrdinalMap> ordinalMap,
      int64_t valueCount, int min, int max);

protected:
  void doScore(int globalOrd, float existingScore, float newScore) override;

  float unset() override;

protected:
  std::shared_ptr<Min> shared_from_this()
  {
    return std::static_pointer_cast<Min>(
        GlobalOrdinalsWithScoreCollector::shared_from_this());
  }
};
class GlobalOrdinalsWithScoreCollector::Max final
    : public GlobalOrdinalsWithScoreCollector
{
  GET_CLASS_NAME(GlobalOrdinalsWithScoreCollector::Max)

public:
  Max(const std::wstring &field, std::shared_ptr<OrdinalMap> ordinalMap,
      int64_t valueCount, int min, int max);

protected:
  void doScore(int globalOrd, float existingScore, float newScore) override;

  float unset() override;

protected:
  std::shared_ptr<Max> shared_from_this()
  {
    return std::static_pointer_cast<Max>(
        GlobalOrdinalsWithScoreCollector::shared_from_this());
  }
};
class GlobalOrdinalsWithScoreCollector::Sum final
    : public GlobalOrdinalsWithScoreCollector
{
  GET_CLASS_NAME(GlobalOrdinalsWithScoreCollector::Sum)

public:
  Sum(const std::wstring &field, std::shared_ptr<OrdinalMap> ordinalMap,
      int64_t valueCount, int min, int max);

protected:
  void doScore(int globalOrd, float existingScore, float newScore) override;

  float unset() override;

protected:
  std::shared_ptr<Sum> shared_from_this()
  {
    return std::static_pointer_cast<Sum>(
        GlobalOrdinalsWithScoreCollector::shared_from_this());
  }
};
class GlobalOrdinalsWithScoreCollector::Avg final
    : public GlobalOrdinalsWithScoreCollector
{
  GET_CLASS_NAME(GlobalOrdinalsWithScoreCollector::Avg)

public:
  Avg(const std::wstring &field, std::shared_ptr<OrdinalMap> ordinalMap,
      int64_t valueCount, int min, int max);

protected:
  void doScore(int globalOrd, float existingScore, float newScore) override;

public:
  float score(int globalOrdinal) override;

protected:
  float unset() override;

protected:
  std::shared_ptr<Avg> shared_from_this()
  {
    return std::static_pointer_cast<Avg>(
        GlobalOrdinalsWithScoreCollector::shared_from_this());
  }
};
class GlobalOrdinalsWithScoreCollector::NoScore final
    : public GlobalOrdinalsWithScoreCollector
{
  GET_CLASS_NAME(GlobalOrdinalsWithScoreCollector::NoScore)

public:
  NoScore(const std::wstring &field, std::shared_ptr<OrdinalMap> ordinalMap,
          int64_t valueCount, int min, int max);

  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class LeafCollectorAnonymousInnerClass
      : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass>,
        public LeafCollector
  {
    GET_CLASS_NAME(LeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<NoScore> outerInstance;

    std::shared_ptr<SortedDocValues> docTermOrds;
    std::shared_ptr<LongValues> segmentOrdToGlobalOrdLookup;

  public:
    LeafCollectorAnonymousInnerClass(
        std::shared_ptr<NoScore> outerInstance,
        std::shared_ptr<SortedDocValues> docTermOrds,
        std::shared_ptr<LongValues> segmentOrdToGlobalOrdLookup);

    void setScorer(std::shared_ptr<Scorer> scorer) ;

    void collect(int doc)  override;
  };

private:
  class LeafCollectorAnonymousInnerClass2
      : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass2>,
        public LeafCollector
  {
    GET_CLASS_NAME(LeafCollectorAnonymousInnerClass2)
  private:
    std::shared_ptr<NoScore> outerInstance;

    std::shared_ptr<SortedDocValues> docTermOrds;

  public:
    LeafCollectorAnonymousInnerClass2(
        std::shared_ptr<NoScore> outerInstance,
        std::shared_ptr<SortedDocValues> docTermOrds);

    void setScorer(std::shared_ptr<Scorer> scorer) ;

    void collect(int doc)  override;
  };

protected:
  void doScore(int globalOrd, float existingScore, float newScore) override;

public:
  float score(int globalOrdinal) override;

protected:
  float unset() override;

public:
  bool needsScores() override;

protected:
  std::shared_ptr<NoScore> shared_from_this()
  {
    return std::static_pointer_cast<NoScore>(
        GlobalOrdinalsWithScoreCollector::shared_from_this());
  }
};
