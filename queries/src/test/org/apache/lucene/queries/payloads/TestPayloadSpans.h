#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::search::spans
{
class Spans;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PayloadAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::queries::payloads
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using IndexReader = org::apache::lucene::index::IndexReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using SpanCollector = org::apache::lucene::search::spans::SpanCollector;
using Spans = org::apache::lucene::search::spans::Spans;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestPayloadSpans : public LuceneTestCase
{
  GET_CLASS_NAME(TestPayloadSpans)
private:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<Similarity> similarity =
      std::make_shared<ClassicSimilarity>();

protected:
  std::shared_ptr<IndexReader> indexReader;

private:
  std::shared_ptr<IndexReader> closeIndexReader;
  std::shared_ptr<Directory> directory;

public:
  void setUp()  override;

  virtual void testSpanTermQuery() ;

  virtual void testSpanFirst() ;

  virtual void testSpanNot() ;

  virtual void testNestedSpans() ;

  virtual void testFirstClauseWithoutPayload() ;

  virtual void testHeavilyNestedSpanQuery() ;

  virtual void testShrinkToAfterShortestMatch() ;

  virtual void testShrinkToAfterShortestMatch2() ;

  virtual void testShrinkToAfterShortestMatch3() ;

public:
  class VerifyingCollector
      : public std::enable_shared_from_this<VerifyingCollector>,
        public SpanCollector
  {
    GET_CLASS_NAME(VerifyingCollector)

  public:
    std::deque<std::shared_ptr<BytesRef>> payloads =
        std::deque<std::shared_ptr<BytesRef>>();

    void collectLeaf(std::shared_ptr<PostingsEnum> postings, int position,
                     std::shared_ptr<Term> term)  override;

    void reset() override;

    virtual void verify(int expectedLength, int expectedFirstByte);
  };

private:
  void checkSpans(std::shared_ptr<Spans> spans, int expectedNumSpans,
                  int expectedNumPayloads, int expectedPayloadLength,
                  int expectedFirstByte) ;

  std::shared_ptr<IndexSearcher> getSearcher() ;

  void checkSpans(std::shared_ptr<Spans> spans, int numSpans,
                  std::deque<int> &numPayloads) ;

public:
  class PayloadAnalyzer final : public Analyzer
  {
    GET_CLASS_NAME(PayloadAnalyzer)

  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<PayloadAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<PayloadAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  class PayloadFilter final : public TokenFilter
  {
    GET_CLASS_NAME(PayloadFilter)
  public:
    std::shared_ptr<Set<std::wstring>> entities =
        std::unordered_set<std::wstring>();
    std::shared_ptr<Set<std::wstring>> nopayload =
        std::unordered_set<std::wstring>();
    int pos = 0;
    std::shared_ptr<PayloadAttribute> payloadAtt;
    std::shared_ptr<CharTermAttribute> termAtt;
    std::shared_ptr<PositionIncrementAttribute> posIncrAtt;

    PayloadFilter(std::shared_ptr<TokenStream> input);

    bool incrementToken()  override;

    void reset()  override;

  protected:
    std::shared_ptr<PayloadFilter> shared_from_this()
    {
      return std::static_pointer_cast<PayloadFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

public:
  class TestPayloadAnalyzer final : public Analyzer
  {
    GET_CLASS_NAME(TestPayloadAnalyzer)

  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<TestPayloadAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<TestPayloadAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestPayloadSpans> shared_from_this()
  {
    return std::static_pointer_cast<TestPayloadSpans>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::payloads
