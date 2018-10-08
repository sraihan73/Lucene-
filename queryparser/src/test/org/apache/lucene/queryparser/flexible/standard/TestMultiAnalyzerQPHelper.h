#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/QueryNodeException.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"

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
namespace org::apache::lucene::queryparser::flexible::standard
{

using namespace org::apache::lucene::analysis;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * This test case is a copy of the core Lucene query parser test, it was adapted
 * to use new QueryParserHelper instead of the old query parser.
 *
 * Test QueryParser's ability to deal with Analyzers that return more than one
 * token per position or that return tokens with a position increment &gt; 1.
 */
class TestMultiAnalyzerQPHelper : public LuceneTestCase
{
  GET_CLASS_NAME(TestMultiAnalyzerQPHelper)

private:
  static int multiToken;

public:
  virtual void testMultiAnalyzer() ;

  // public void testMultiAnalyzerWithSubclassOfQueryParser() throws
  // ParseException {
  GET_CLASS_NAME(OfQueryParser())
  // this test doesn't make sense when using the new QueryParser API
  // DumbQueryParser qp = new DumbQueryParser("", new MultiAnalyzer());
  // qp.setPhraseSlop(99); // modified default slop
  //
  // // direct call to (super's) getFieldQuery to demonstrate differnce
  // // between phrase and multiphrase with modified default slop
  // assertEquals("\"foo bar\"~99",
  // qp.getSuperFieldQuery("","foo bar").toString());
  // assertEquals("\"(multi multi2) bar\"~99",
  // qp.getSuperFieldQuery("","multi bar").toString());
  //
  //
  // // ask sublcass to parse phrase with modified default slop
  // assertEquals("\"(multi multi2) foo\"~99 bar",
  // qp.parse("\"multi foo\" bar").toString());
  //
  // }

  virtual void testPosIncrementAnalyzer() ;

  /**
   * Expands "multi" to "multi" and "multi2", both at the same position, and
   * expands "triplemulti" to "triplemulti", "multi3", and "multi2".
   */
private:
  class MultiAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(MultiAnalyzer)

  public:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<MultiAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<MultiAnalyzer>(
          Analyzer::shared_from_this());
    }
  };

private:
  class TestFilter final : public TokenFilter
  {
    GET_CLASS_NAME(TestFilter)

  private:
    std::wstring prevType;
    int prevStartOffset = 0;
    int prevEndOffset = 0;

    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
        addAttribute(PositionIncrementAttribute::typeid);
    const std::shared_ptr<OffsetAttribute> offsetAtt =
        addAttribute(OffsetAttribute::typeid);
    const std::shared_ptr<TypeAttribute> typeAtt =
        addAttribute(TypeAttribute::typeid);

  public:
    TestFilter(std::shared_ptr<TokenStream> in_);

    bool incrementToken()  override final;

    void reset()  override;

  protected:
    std::shared_ptr<TestFilter> shared_from_this()
    {
      return std::static_pointer_cast<TestFilter>(
          TokenFilter::shared_from_this());
    }
  };

  /**
   * Analyzes "the quick brown" as: quick(incr=2) brown(incr=1). Does not work
   * correctly for input other than "the quick brown ...".
   */
private:
  class PosIncrementAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(PosIncrementAnalyzer)

  public:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<PosIncrementAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<PosIncrementAnalyzer>(
          Analyzer::shared_from_this());
    }
  };

private:
  class TestPosIncrementFilter : public TokenFilter
  {
    GET_CLASS_NAME(TestPosIncrementFilter)

  private:
    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
        addAttribute(PositionIncrementAttribute::typeid);

  public:
    TestPosIncrementFilter(std::shared_ptr<TokenStream> in_);

    bool incrementToken()  override final;

  protected:
    std::shared_ptr<TestPosIncrementFilter> shared_from_this()
    {
      return std::static_pointer_cast<TestPosIncrementFilter>(
          TokenFilter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestMultiAnalyzerQPHelper> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiAnalyzerQPHelper>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/
