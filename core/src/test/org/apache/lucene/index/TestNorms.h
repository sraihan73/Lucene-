#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

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
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Test that norms info is preserved during index life - including
 * separate norms, addDocument, addIndexes, forceMerge.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({ "Memory", "Direct", "SimpleText" }) @Slow
// public class TestNorms extends org.apache.lucene.util.LuceneTestCase
class TestNorms : public LuceneTestCase
{
public:
  static const std::wstring BYTE_TEST_FIELD;

  virtual void testMaxByteNorms() ;

  // TODO: create a testNormsNotPresent ourselves by adding/deleting/merging
  // docs

  virtual void buildIndex(std::shared_ptr<Directory> dir) ;

public:
  class MySimProvider : public PerFieldSimilarityWrapper
  {
    GET_CLASS_NAME(MySimProvider)
  private:
    std::shared_ptr<TestNorms> outerInstance;

  public:
    MySimProvider(std::shared_ptr<TestNorms> outerInstance);

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
  class ByteEncodingBoostSimilarity : public Similarity
  {
    GET_CLASS_NAME(ByteEncodingBoostSimilarity)

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
    std::shared_ptr<ByteEncodingBoostSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<ByteEncodingBoostSimilarity>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

public:
  virtual void testEmptyValueVsNoValue() ;

protected:
  std::shared_ptr<TestNorms> shared_from_this()
  {
    return std::static_pointer_cast<TestNorms>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
