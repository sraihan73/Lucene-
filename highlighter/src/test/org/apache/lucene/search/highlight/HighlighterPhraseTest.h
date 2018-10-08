#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/highlight/InvalidTokenOffsetsException.h"

#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/analysis/Token.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

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
namespace org::apache::lucene::search::highlight
{

using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class HighlighterPhraseTest : public LuceneTestCase
{
  GET_CLASS_NAME(HighlighterPhraseTest)
private:
  static const std::wstring FIELD;

public:
  virtual void testConcurrentPhrase() throw(IOException,
                                            InvalidTokenOffsetsException);

  virtual void testConcurrentSpan() throw(IOException,
                                          InvalidTokenOffsetsException);

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<HighlighterPhraseTest> outerInstance;

    std::shared_ptr<FixedBitSet> bitset;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<HighlighterPhraseTest> outerInstance,
        std::shared_ptr<FixedBitSet> bitset);

  private:
    int baseDoc = 0;

  public:
    void collect(int i) override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void setScorer(
        std::shared_ptr<org::apache::lucene::search::Scorer> scorer) override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          org.apache.lucene.search.SimpleCollector::shared_from_this());
    }
  };

public:
  virtual void testSparsePhrase() throw(IOException,
                                        InvalidTokenOffsetsException);

  virtual void
  testSparsePhraseWithNoPositions() throw(IOException,
                                          InvalidTokenOffsetsException);

  virtual void testSparseSpan() throw(IOException,
                                      InvalidTokenOffsetsException);

  // shows the need to sum the increments in WeightedSpanTermExtractor
  virtual void testStopWords() ;

  // shows the need to require inOrder if getSlop() == 0, not if final slop == 0
  // in WeightedSpanTermExtractor
  virtual void testInOrderWithStopWords() throw(IOException,
                                                InvalidTokenOffsetsException);

private:
  class TokenStreamSparse final : public TokenStream
  {
    GET_CLASS_NAME(TokenStreamSparse)
  private:
    std::deque<std::shared_ptr<Token>> tokens;

    int i = -1;

    const std::shared_ptr<CharTermAttribute> termAttribute =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<OffsetAttribute> offsetAttribute =
        addAttribute(OffsetAttribute::typeid);
    const std::shared_ptr<PositionIncrementAttribute>
        positionIncrementAttribute =
            addAttribute(PositionIncrementAttribute::typeid);

  public:
    TokenStreamSparse();

    bool incrementToken() override;

    void reset() override;

  protected:
    std::shared_ptr<TokenStreamSparse> shared_from_this()
    {
      return std::static_pointer_cast<TokenStreamSparse>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

private:
  class TokenStreamConcurrent final : public TokenStream
  {
    GET_CLASS_NAME(TokenStreamConcurrent)
  private:
    std::deque<std::shared_ptr<Token>> tokens;

    int i = -1;

    const std::shared_ptr<CharTermAttribute> termAttribute =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<OffsetAttribute> offsetAttribute =
        addAttribute(OffsetAttribute::typeid);
    const std::shared_ptr<PositionIncrementAttribute>
        positionIncrementAttribute =
            addAttribute(PositionIncrementAttribute::typeid);

  public:
    TokenStreamConcurrent();

    bool incrementToken() override;

    void reset() override;

  protected:
    std::shared_ptr<TokenStreamConcurrent> shared_from_this()
    {
      return std::static_pointer_cast<TokenStreamConcurrent>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

protected:
  std::shared_ptr<HighlighterPhraseTest> shared_from_this()
  {
    return std::static_pointer_cast<HighlighterPhraseTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/highlight/
