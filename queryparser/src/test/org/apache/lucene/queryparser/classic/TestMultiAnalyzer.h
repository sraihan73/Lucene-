#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::classic
{
class ParseException;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
}
namespace org::apache::lucene::search
{
class Query;
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
namespace org::apache::lucene::queryparser::classic
{

using namespace org::apache::lucene::analysis;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using Query = org::apache::lucene::search::Query;

/**
 * Test QueryParser's ability to deal with Analyzers that return more
 * than one token per position or that return tokens with a position
 * increment &gt; 1.
 *
 */
class TestMultiAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestMultiAnalyzer)

private:
  static int multiToken;

public:
  virtual void testMultiAnalyzer() ;

  virtual void
  testMultiAnalyzerWithSubclassOfQueryParser() ;

  virtual void testPosIncrementAnalyzer() ;

  /**
   * Expands "multi" to "multi" and "multi2", both at the same position,
   * and expands "triplemulti" to "triplemulti", "multi3", and "multi2".
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

    const std::shared_ptr<CharTermAttribute> termAtt;
    const std::shared_ptr<PositionIncrementAttribute> posIncrAtt;
    const std::shared_ptr<OffsetAttribute> offsetAtt;
    const std::shared_ptr<TypeAttribute> typeAtt;

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
   * Analyzes "the quick brown" as: quick(incr=2) brown(incr=1).
   * Does not work correctly for input other than "the quick brown ...".
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
  class TestPosIncrementFilter final : public TokenFilter
  {
    GET_CLASS_NAME(TestPosIncrementFilter)

  public:
    std::shared_ptr<CharTermAttribute> termAtt;
    std::shared_ptr<PositionIncrementAttribute> posIncrAtt;

    TestPosIncrementFilter(std::shared_ptr<TokenStream> in_);

    bool incrementToken()  override final;

  protected:
    std::shared_ptr<TestPosIncrementFilter> shared_from_this()
    {
      return std::static_pointer_cast<TestPosIncrementFilter>(
          TokenFilter::shared_from_this());
    }
  };

  /** a very simple subclass of QueryParser */
private:
  class DumbQueryParser final : public QueryParser
  {
    GET_CLASS_NAME(DumbQueryParser)

  public:
    DumbQueryParser(const std::wstring &f, std::shared_ptr<Analyzer> a);

    /** expose super's version */
    std::shared_ptr<Query>
    getSuperFieldQuery(const std::wstring &f, const std::wstring &t,
                       bool quoted) ;
    /** wrap super's version */
  protected:
    std::shared_ptr<Query>
    getFieldQuery(const std::wstring &f, const std::wstring &t,
                  bool quoted)  override;

  protected:
    std::shared_ptr<DumbQueryParser> shared_from_this()
    {
      return std::static_pointer_cast<DumbQueryParser>(
          QueryParser::shared_from_this());
    }
  };

  /**
   * A very simple wrapper to prevent instanceof checks but uses
   * the toString of the query it wraps.
   */
private:
  class DumbQueryWrapper final : public Query
  {
    GET_CLASS_NAME(DumbQueryWrapper)
  private:
    std::shared_ptr<Query> q;

  public:
    DumbQueryWrapper(std::shared_ptr<Query> q);
    std::wstring toString(const std::wstring &f) override;

    bool equals(std::any other) override;

    virtual int hashCode();

  protected:
    std::shared_ptr<DumbQueryWrapper> shared_from_this()
    {
      return std::static_pointer_cast<DumbQueryWrapper>(
          org.apache.lucene.search.Query::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestMultiAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiAnalyzer>(
        BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::classic
