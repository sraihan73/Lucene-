#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::similarities
{
class Similarity;
}

namespace org::apache::lucene::index
{
class FieldInvertState;
}
namespace org::apache::lucene::search
{
class CollectionStatistics;
}
namespace org::apache::lucene::search
{
class TermStatistics;
}
namespace org::apache::lucene::search::similarities
{
class SimWeight;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
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
namespace org::apache::lucene::index
{

using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using PerFieldSimilarityWrapper =
    org::apache::lucene::search::similarities::PerFieldSimilarityWrapper;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 *
 */
class TestCustomNorms : public LuceneTestCase
{
  GET_CLASS_NAME(TestCustomNorms)
public:
  static const std::wstring FLOAT_TEST_FIELD;
  static const std::wstring EXCEPTION_TEST_FIELD;

  virtual void testFloatNorms() ;

public:
  class MySimProvider : public PerFieldSimilarityWrapper
  {
    GET_CLASS_NAME(MySimProvider)
  private:
    std::shared_ptr<TestCustomNorms> outerInstance;

  public:
    MySimProvider(std::shared_ptr<TestCustomNorms> outerInstance);

    std::shared_ptr<Similarity> delegate_ =
        std::make_shared<ClassicSimilarity>();

    std::shared_ptr<Similarity> get(const std::wstring &field) override;

  protected:
    std::shared_ptr<MySimProvider> shared_from_this()
    {
      return std::static_pointer_cast<MySimProvider>(
          org.apache.lucene.search.similarities
              .PerFieldSimilarityWrapper::shared_from_this());
    }
  };

public:
  class FloatEncodingBoostSimilarity : public Similarity
  {
    GET_CLASS_NAME(FloatEncodingBoostSimilarity)

  public:
    int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override;

    std::shared_ptr<Similarity::SimWeight>
    computeWeight(float boost,
                  std::shared_ptr<CollectionStatistics> collectionStats,
                  std::deque<TermStatistics> &termStats) override;

    std::shared_ptr<Similarity::SimScorer> simScorer(
        std::shared_ptr<Similarity::SimWeight> weight,
        std::shared_ptr<LeafReaderContext> context)  override;

  protected:
    std::shared_ptr<FloatEncodingBoostSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<FloatEncodingBoostSimilarity>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestCustomNorms> shared_from_this()
  {
    return std::static_pointer_cast<TestCustomNorms>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
