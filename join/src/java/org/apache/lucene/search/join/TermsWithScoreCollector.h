#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"

#include  "core/src/java/org/apache/lucene/search/join/Avg.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"

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

using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Scorer = org::apache::lucene::search::Scorer;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;

template <typename DV>
class TermsWithScoreCollector : public DocValuesTermsCollector<DV>,
                                public GenericTermsCollector
{
  GET_CLASS_NAME(TermsWithScoreCollector)

private:
  static constexpr int INITIAL_ARRAY_SIZE = 0;

public:
  const std::shared_ptr<BytesRefHash> collectedTerms =
      std::make_shared<BytesRefHash>();
  const ScoreMode scoreMode;

  std::shared_ptr<Scorer> scorer;
  std::deque<float> scoreSums = std::deque<float>(INITIAL_ARRAY_SIZE);

  TermsWithScoreCollector(Function<DV> docValuesCall, ScoreMode scoreMode)
      : DocValuesTermsCollector<DV>(docValuesCall), scoreMode(scoreMode)
  {
    if (scoreMode == ScoreMode::Min) {
      Arrays::fill(scoreSums, std::numeric_limits<float>::infinity());
    } else if (scoreMode == ScoreMode::Max) {
      Arrays::fill(scoreSums, -std::numeric_limits<float>::infinity());
    }
  }

  std::shared_ptr<BytesRefHash> getCollectedTerms() override
  {
    return collectedTerms;
  }

  std::deque<float> getScoresPerTerm() override { return scoreSums; }

  void setScorer(std::shared_ptr<Scorer> scorer)  override
  {
    this->scorer = scorer;
  }

  /**
   * Chooses the right {@link TermsWithScoreCollector} implementation.
   *
   * @param field                     The field to collect terms for
   * @param multipleValuesPerDocument Whether the field to collect terms for has
   * multiple values per document.
   * @return a {@link TermsWithScoreCollector} instance
   */
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: static TermsWithScoreCollector<?> create(std::wstring field,
  // bool multipleValuesPerDocument, ScoreMode scoreMode)
  static std::shared_ptr < TermsWithScoreCollector <
      ? >> create(const std::wstring &field, bool multipleValuesPerDocument,
                  ScoreMode scoreMode)
  {
    if (multipleValuesPerDocument) {
      switch (scoreMode) {
      case org::apache::lucene::search::join::ScoreMode::Avg:
        return std::make_shared<MV::Avg>(sortedSetDocValues(field));
      default:
        return std::make_shared<MV>(sortedSetDocValues(field), scoreMode);
      }
    } else {
      switch (scoreMode) {
      case org::apache::lucene::search::join::ScoreMode::Avg:
        return std::make_shared<SV::Avg>(binaryDocValues(field));
      default:
        return std::make_shared<SV>(binaryDocValues(field), scoreMode);
      }
    }
  }

  // impl that works with single value per document
public:
  class SV : public TermsWithScoreCollector<std::shared_ptr<BinaryDocValues>>
  {
    GET_CLASS_NAME(SV)

  public:
    SV(Function<std::shared_ptr<BinaryDocValues>> docValuesCall,
       ScoreMode scoreMode);

    void collect(int doc)  override;

  public:
    class Avg : public SV
    {
      GET_CLASS_NAME(Avg)

    public:
      std::deque<int> scoreCounts = std::deque<int>(INITIAL_ARRAY_SIZE);

      Avg(Function<std::shared_ptr<BinaryDocValues>> docValuesCall);

      void collect(int doc)  override;

      std::deque<float> getScoresPerTerm() override;

    protected:
      std::shared_ptr<Avg> shared_from_this()
      {
        return std::static_pointer_cast<Avg>(SV::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<SV> shared_from_this()
    {
      return std::static_pointer_cast<SV>(
          TermsWithScoreCollector<
              org.apache.lucene.index.BinaryDocValues>::shared_from_this());
    }
  };

  // impl that works with multiple values per document
public:
  class MV : public TermsWithScoreCollector<std::shared_ptr<SortedSetDocValues>>
  {
    GET_CLASS_NAME(MV)

  public:
    MV(Function<std::shared_ptr<SortedSetDocValues>> docValuesCall,
       ScoreMode scoreMode);

    void collect(int doc)  override;

  public:
    class Avg : public MV
    {
      GET_CLASS_NAME(Avg)

    public:
      std::deque<int> scoreCounts = std::deque<int>(INITIAL_ARRAY_SIZE);

      Avg(Function<std::shared_ptr<SortedSetDocValues>> docValuesCall);

      void collect(int doc)  override;

      std::deque<float> getScoresPerTerm() override;

    protected:
      std::shared_ptr<Avg> shared_from_this()
      {
        return std::static_pointer_cast<Avg>(MV::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<MV> shared_from_this()
    {
      return std::static_pointer_cast<MV>(
          TermsWithScoreCollector<
              org.apache.lucene.index.SortedSetDocValues>::shared_from_this());
    }
  };

public:
  bool needsScores() override { return true; }

protected:
  std::shared_ptr<TermsWithScoreCollector> shared_from_this()
  {
    return std::static_pointer_cast<TermsWithScoreCollector>(
        DocValuesTermsCollector<DV>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/join/
