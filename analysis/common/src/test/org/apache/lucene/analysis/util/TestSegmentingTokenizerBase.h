#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/SegmentingTokenizerBase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
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
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
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
namespace org::apache::lucene::analysis::util
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/** Basic tests for {@link SegmentingTokenizerBase} */
class TestSegmentingTokenizerBase : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestSegmentingTokenizerBase)
private:
  std::shared_ptr<Analyzer> sentence, sentenceAndWord;

public:
  void setUp()  override;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestSegmentingTokenizerBase> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestSegmentingTokenizerBase> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestSegmentingTokenizerBase> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestSegmentingTokenizerBase> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  void tearDown()  override;

  /** Some simple examples, just outputting the whole sentence boundaries as
   * "terms" */
  virtual void testBasics() ;

  /** Test a subclass that sets some custom attribute values */
  virtual void testCustomAttributes() ;

  /** Tests tokenstream reuse */
  virtual void testReuse() ;

  /** Tests TokenStream.end() */
  virtual void testEnd() ;

  /** Tests terms which span across boundaries */
  virtual void testHugeDoc() ;

  /** Tests the handling of binary/malformed data */
  virtual void testHugeTerm() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

  // some tokenizers for testing

  /** silly tokenizer that just returns whole sentences as tokens */
public:
  class WholeSentenceTokenizer : public SegmentingTokenizerBase
  {
    GET_CLASS_NAME(WholeSentenceTokenizer)
  public:
    int sentenceStart = 0, sentenceEnd = 0;
    bool hasSentence = false;

  private:
    std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    std::shared_ptr<OffsetAttribute> offsetAtt =
        addAttribute(OffsetAttribute::typeid);

  public:
    WholeSentenceTokenizer();

  protected:
    void setNextSentence(int sentenceStart, int sentenceEnd) override;

    bool incrementWord() override;

  protected:
    std::shared_ptr<WholeSentenceTokenizer> shared_from_this()
    {
      return std::static_pointer_cast<WholeSentenceTokenizer>(
          SegmentingTokenizerBase::shared_from_this());
    }
  };

  /**
   * simple tokenizer, that bumps posinc + 1 for tokens after a
   * sentence boundary to inhibit phrase queries without slop.
   */
public:
  class SentenceAndWordTokenizer : public SegmentingTokenizerBase
  {
    GET_CLASS_NAME(SentenceAndWordTokenizer)
  public:
    int sentenceStart = 0, sentenceEnd = 0;
    int wordStart = 0, wordEnd = 0;
    int posBoost = -1; // initially set to -1 so the first word in the document
                       // doesn't get a pos boost

  private:
    std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    std::shared_ptr<OffsetAttribute> offsetAtt =
        addAttribute(OffsetAttribute::typeid);
    std::shared_ptr<PositionIncrementAttribute> posIncAtt =
        addAttribute(PositionIncrementAttribute::typeid);

  public:
    SentenceAndWordTokenizer();

  protected:
    void setNextSentence(int sentenceStart, int sentenceEnd) override;

  public:
    void reset()  override;

  protected:
    bool incrementWord() override;

  protected:
    std::shared_ptr<SentenceAndWordTokenizer> shared_from_this()
    {
      return std::static_pointer_cast<SentenceAndWordTokenizer>(
          SegmentingTokenizerBase::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestSegmentingTokenizerBase> shared_from_this()
  {
    return std::static_pointer_cast<TestSegmentingTokenizerBase>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::util
