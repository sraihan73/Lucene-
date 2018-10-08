#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class CollectionStatistics;
}

namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::search
{
class TermStatistics;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
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
namespace org::apache::lucene::search
{

using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Similarity unit test.
 *
 *
 */
class TestSimilarity : public LuceneTestCase
{
  GET_CLASS_NAME(TestSimilarity)

public:
  class SimpleSimilarity : public ClassicSimilarity
  {
    GET_CLASS_NAME(SimpleSimilarity)
  public:
    float lengthNorm(int length) override;
    float tf(float freq) override;
    float sloppyFreq(int distance) override;
    float idf(int64_t docFreq, int64_t docCount) override;
    std::shared_ptr<Explanation>
    idfExplain(std::shared_ptr<CollectionStatistics> collectionStats,
               std::deque<std::shared_ptr<TermStatistics>> &stats) override;

  protected:
    std::shared_ptr<SimpleSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<SimpleSimilarity>(
          org.apache.lucene.search.similarities
              .ClassicSimilarity::shared_from_this());
    }
  };

public:
  virtual void testSimilarity() ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestSimilarity> outerInstance;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<TestSimilarity> outerInstance);

  private:
    std::shared_ptr<Scorer> scorer;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer) override;
    void collect(int doc)  override final;
    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          SimpleCollector::shared_from_this());
    }
  };

private:
  class SimpleCollectorAnonymousInnerClass2 : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass2)
  private:
    std::shared_ptr<TestSimilarity> outerInstance;

  public:
    SimpleCollectorAnonymousInnerClass2(
        std::shared_ptr<TestSimilarity> outerInstance);

  private:
    int base = 0;
    std::shared_ptr<Scorer> scorer;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer) override;
    void collect(int doc)  override final;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass2>(
          SimpleCollector::shared_from_this());
    }
  };

private:
  class SimpleCollectorAnonymousInnerClass3 : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass3)
  private:
    std::shared_ptr<TestSimilarity> outerInstance;

  public:
    SimpleCollectorAnonymousInnerClass3(
        std::shared_ptr<TestSimilarity> outerInstance);

  private:
    std::shared_ptr<Scorer> scorer;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer) override;
    void collect(int doc)  override final;
    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass3>(
          SimpleCollector::shared_from_this());
    }
  };

private:
  class SimpleCollectorAnonymousInnerClass4 : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass4)
  private:
    std::shared_ptr<TestSimilarity> outerInstance;

  public:
    SimpleCollectorAnonymousInnerClass4(
        std::shared_ptr<TestSimilarity> outerInstance);

  private:
    std::shared_ptr<Scorer> scorer;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer) override;
    void collect(int doc)  override final;
    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass4>(
          SimpleCollector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestSimilarity> shared_from_this()
  {
    return std::static_pointer_cast<TestSimilarity>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
