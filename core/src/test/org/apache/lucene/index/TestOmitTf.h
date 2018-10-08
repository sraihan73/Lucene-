#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
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
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::document
{
class FieldType;
}
namespace org::apache::lucene::store
{
class Directory;
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
namespace org::apache::lucene::index
{

using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using TFIDFSimilarity =
    org::apache::lucene::search::similarities::TFIDFSimilarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestOmitTf : public LuceneTestCase
{
  GET_CLASS_NAME(TestOmitTf)

public:
  class SimpleSimilarity : public TFIDFSimilarity
  {
    GET_CLASS_NAME(SimpleSimilarity)
  public:
    float lengthNorm(int length) override;
    float tf(float freq) override;
    float sloppyFreq(int distance) override;
    float idf(int64_t docFreq, int64_t docCount) override;
    std::shared_ptr<Explanation> idfExplain(
        std::shared_ptr<CollectionStatistics> collectionStats,
        std::deque<std::shared_ptr<TermStatistics>> &termStats) override;
    float scorePayload(int doc, int start, int end,
                       std::shared_ptr<BytesRef> payload) override;

  protected:
    std::shared_ptr<SimpleSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<SimpleSimilarity>(
          org.apache.lucene.search.similarities
              .TFIDFSimilarity::shared_from_this());
    }
  };

private:
  static const std::shared_ptr<FieldType> omitType;
  static const std::shared_ptr<FieldType> normalType;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static TestOmitTf::StaticConstructor staticConstructor;

  // Tests whether the DocumentWriter correctly enable the
  // omitTermFreqAndPositions bit in the FieldInfo
public:
  virtual void testOmitTermFreqAndPositions() ;

  // Tests whether merging of docs that have different
  // omitTermFreqAndPositions for the same field works
  virtual void testMixedMerge() ;

  // Make sure first adding docs that do not omitTermFreqAndPositions for
  // field X, then adding docs that do omitTermFreqAndPositions for that same
  // field,
  virtual void testMixedRAM() ;

private:
  void assertNoPrx(std::shared_ptr<Directory> dir) ;

  // Verifies no *.prx exists when all fields omit term freq:
public:
  virtual void testNoPrxFile() ;

  // Test scores with one field with Term Freqs and one without, otherwise with
  // equal content
  virtual void testBasic() ;

private:
  class CountingHitCollectorAnonymousInnerClass : public CountingHitCollector
  {
    GET_CLASS_NAME(CountingHitCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestOmitTf> outerInstance;

  public:
    CountingHitCollectorAnonymousInnerClass(
        std::shared_ptr<TestOmitTf> outerInstance);

  private:
    std::shared_ptr<Scorer> scorer;

  public:
    bool needsScores() override;
    void setScorer(std::shared_ptr<Scorer> scorer) override final;
    void collect(int doc)  override final;

  protected:
    std::shared_ptr<CountingHitCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CountingHitCollectorAnonymousInnerClass>(
          CountingHitCollector::shared_from_this());
    }
  };

private:
  class CountingHitCollectorAnonymousInnerClass2 : public CountingHitCollector
  {
    GET_CLASS_NAME(CountingHitCollectorAnonymousInnerClass2)
  private:
    std::shared_ptr<TestOmitTf> outerInstance;

  public:
    CountingHitCollectorAnonymousInnerClass2(
        std::shared_ptr<TestOmitTf> outerInstance);

  private:
    std::shared_ptr<Scorer> scorer;

  public:
    bool needsScores() override;
    void setScorer(std::shared_ptr<Scorer> scorer) override final;
    void collect(int doc)  override final;

  protected:
    std::shared_ptr<CountingHitCollectorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<CountingHitCollectorAnonymousInnerClass2>(
          CountingHitCollector::shared_from_this());
    }
  };

private:
  class CountingHitCollectorAnonymousInnerClass3 : public CountingHitCollector
  {
    GET_CLASS_NAME(CountingHitCollectorAnonymousInnerClass3)
  private:
    std::shared_ptr<TestOmitTf> outerInstance;

  public:
    CountingHitCollectorAnonymousInnerClass3(
        std::shared_ptr<TestOmitTf> outerInstance);

  private:
    std::shared_ptr<Scorer> scorer;

  public:
    bool needsScores() override;
    void setScorer(std::shared_ptr<Scorer> scorer) override final;
    void collect(int doc)  override final;

  protected:
    std::shared_ptr<CountingHitCollectorAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<CountingHitCollectorAnonymousInnerClass3>(
          CountingHitCollector::shared_from_this());
    }
  };

private:
  class CountingHitCollectorAnonymousInnerClass4 : public CountingHitCollector
  {
    GET_CLASS_NAME(CountingHitCollectorAnonymousInnerClass4)
  private:
    std::shared_ptr<TestOmitTf> outerInstance;

  public:
    CountingHitCollectorAnonymousInnerClass4(
        std::shared_ptr<TestOmitTf> outerInstance);

  private:
    std::shared_ptr<Scorer> scorer;

  public:
    bool needsScores() override;
    void setScorer(std::shared_ptr<Scorer> scorer) override final;
    void collect(int doc)  override final;

  protected:
    std::shared_ptr<CountingHitCollectorAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<CountingHitCollectorAnonymousInnerClass4>(
          CountingHitCollector::shared_from_this());
    }
  };

private:
  class CountingHitCollectorAnonymousInnerClass5 : public CountingHitCollector
  {
    GET_CLASS_NAME(CountingHitCollectorAnonymousInnerClass5)
  private:
    std::shared_ptr<TestOmitTf> outerInstance;

  public:
    CountingHitCollectorAnonymousInnerClass5(
        std::shared_ptr<TestOmitTf> outerInstance);

    void collect(int doc)  override final;

  protected:
    std::shared_ptr<CountingHitCollectorAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<CountingHitCollectorAnonymousInnerClass5>(
          CountingHitCollector::shared_from_this());
    }
  };

public:
  class CountingHitCollector : public SimpleCollector
  {
    GET_CLASS_NAME(CountingHitCollector)
  public:
    static int count;
    static int sum;

  private:
    int docBase = -1;

  public:
    CountingHitCollector();
    void collect(int doc)  override;

    static int getCount();
    static int getSum();

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    bool needsScores() override;

  protected:
    std::shared_ptr<CountingHitCollector> shared_from_this()
    {
      return std::static_pointer_cast<CountingHitCollector>(
          org.apache.lucene.search.SimpleCollector::shared_from_this());
    }
  };

  /** test that when freqs are omitted, that totalTermFreq and sumTotalTermFreq
   * are -1 */
public:
  virtual void testStats() ;

protected:
  std::shared_ptr<TestOmitTf> shared_from_this()
  {
    return std::static_pointer_cast<TestOmitTf>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
