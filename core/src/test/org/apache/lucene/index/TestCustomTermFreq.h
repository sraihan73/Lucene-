#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class TermFrequencyAttribute;
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
class Similarity;
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

using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TermFrequencyAttribute =
    org::apache::lucene::analysis::tokenattributes::TermFrequencyAttribute;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.PostingsEnum.NO_MORE_DOCS;

class TestCustomTermFreq : public LuceneTestCase
{
  GET_CLASS_NAME(TestCustomTermFreq)

private:
  class CannedTermFreqs final : public TokenStream
  {
    GET_CLASS_NAME(CannedTermFreqs)
  private:
    std::deque<std::wstring> const terms;
    std::deque<int> const termFreqs;
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<TermFrequencyAttribute> termFreqAtt =
        addAttribute(TermFrequencyAttribute::typeid);
    int upto = 0;

  public:
    CannedTermFreqs(std::deque<std::wstring> &terms,
                    std::deque<int> &termFreqs);

    bool incrementToken() override;

    void reset() override;

  protected:
    std::shared_ptr<CannedTermFreqs> shared_from_this()
    {
      return std::static_pointer_cast<CannedTermFreqs>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

public:
  virtual void testSingletonTermsOneDoc() ;

  virtual void testSingletonTermsTwoDocs() ;

  virtual void testRepeatTermsOneDoc() ;

  virtual void testRepeatTermsTwoDocs() ;

  virtual void testTotalTermFreq() ;

  // you can't index proximity with custom term freqs:
  virtual void testInvalidProx() ;

  // you can't index DOCS_ONLY with custom term freq
  virtual void testInvalidDocsOnly() ;

  // sum of term freqs must fit in an int
  virtual void testOverflowInt() ;

  virtual void testInvalidTermVectorPositions() ;

  virtual void testInvalidTermVectorOffsets() ;

  virtual void testTermVectors() ;

  /**
   * Similarity holds onto the FieldInvertState for subsequent verification.
   */
private:
  class NeverForgetsSimilarity : public Similarity
  {
    GET_CLASS_NAME(NeverForgetsSimilarity)
  public:
    std::shared_ptr<FieldInvertState> lastState;

  private:
    static const std::shared_ptr<NeverForgetsSimilarity> INSTANCE;

    NeverForgetsSimilarity();

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
    std::shared_ptr<NeverForgetsSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<NeverForgetsSimilarity>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

public:
  virtual void testFieldInvertState() ;

protected:
  std::shared_ptr<TestCustomTermFreq> shared_from_this()
  {
    return std::static_pointer_cast<TestCustomTermFreq>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
