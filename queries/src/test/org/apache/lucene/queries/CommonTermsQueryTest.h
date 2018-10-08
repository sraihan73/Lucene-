#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/BooleanClause.h"

#include  "core/src/java/org/apache/lucene/queries/TermAndFreq.h"
namespace org::apache::lucene::util
{
template <typename T>
class PriorityQueue;
}
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"

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
namespace org::apache::lucene::queries
{

using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

class CommonTermsQueryTest : public LuceneTestCase
{
  GET_CLASS_NAME(CommonTermsQueryTest)

public:
  virtual void testBasics() ;

  virtual void testEqualsHashCode();

private:
  static Occur randomOccur(std::shared_ptr<Random> random);

public:
  virtual void testNullTerm();

  virtual void testMinShouldMatch() ;

  /** MUST_NOT is not supported */
  virtual void testIllegalOccur();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExtend() throws java.io.IOException
  virtual void testExtend() ;

  virtual void testRandomIndex() ;

private:
  class PriorityQueueAnonymousInnerClass
      : public PriorityQueue<std::shared_ptr<CommonTermsQueryTest::TermAndFreq>>
  {
    GET_CLASS_NAME(PriorityQueueAnonymousInnerClass)
  private:
    std::shared_ptr<CommonTermsQueryTest> outerInstance;

  public:
    PriorityQueueAnonymousInnerClass(
        std::shared_ptr<CommonTermsQueryTest> outerInstance);

  protected:
    bool lessThan(std::shared_ptr<TermAndFreq> a,
                  std::shared_ptr<TermAndFreq> b) override;

  protected:
    std::shared_ptr<PriorityQueueAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PriorityQueueAnonymousInnerClass>(
          org.apache.lucene.util.PriorityQueue<
              CommonTermsQueryTest.TermAndFreq>::shared_from_this());
    }
  };

private:
  class PriorityQueueAnonymousInnerClass2
      : public PriorityQueue<std::shared_ptr<CommonTermsQueryTest::TermAndFreq>>
  {
    GET_CLASS_NAME(PriorityQueueAnonymousInnerClass2)
  private:
    std::shared_ptr<CommonTermsQueryTest> outerInstance;

  public:
    PriorityQueueAnonymousInnerClass2(
        std::shared_ptr<CommonTermsQueryTest> outerInstance);

  protected:
    bool lessThan(std::shared_ptr<TermAndFreq> a,
                  std::shared_ptr<TermAndFreq> b) override;

  protected:
    std::shared_ptr<PriorityQueueAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<PriorityQueueAnonymousInnerClass2>(
          org.apache.lucene.util.PriorityQueue<
              CommonTermsQueryTest.TermAndFreq>::shared_from_this());
    }
  };

private:
  static std::deque<std::shared_ptr<TermAndFreq>> queueToList(
      std::shared_ptr<PriorityQueue<std::shared_ptr<TermAndFreq>>> queue);

private:
  class TermAndFreq : public std::enable_shared_from_this<TermAndFreq>
  {
    GET_CLASS_NAME(TermAndFreq)
  public:
    std::shared_ptr<BytesRef> term;
    int freq = 0;

    TermAndFreq(std::shared_ptr<BytesRef> term, int freq);
  };

  /**
   * populates a writer with random stuff. this must be fully reproducable with
   * the seed!
   */
public:
  static void createRandomIndex(int numdocs,
                                std::shared_ptr<RandomIndexWriter> writer,
                                int64_t seed) ;

private:
  class ExtendedCommonTermsQuery final : public CommonTermsQuery
  {
    GET_CLASS_NAME(ExtendedCommonTermsQuery)

  public:
    ExtendedCommonTermsQuery(Occur highFreqOccur, Occur lowFreqOccur,
                             float maxTermFrequency);

  protected:
    std::shared_ptr<Query>
    newTermQuery(std::shared_ptr<Term> term,
                 std::shared_ptr<TermContext> context) override;

  protected:
    std::shared_ptr<ExtendedCommonTermsQuery> shared_from_this()
    {
      return std::static_pointer_cast<ExtendedCommonTermsQuery>(
          CommonTermsQuery::shared_from_this());
    }
  };

protected:
  std::shared_ptr<CommonTermsQueryTest> shared_from_this()
  {
    return std::static_pointer_cast<CommonTermsQueryTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/
