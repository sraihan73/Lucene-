#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::queryparser::flexible::standard
{
class StandardQueryParser;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::queryparser::flexible::standard
{
class CommonQueryParserConfiguration;
}
namespace org::apache::lucene::document
{
class DateTools;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
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
namespace org::apache::lucene::queryparser::flexible::standard
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Resolution = org::apache::lucene::document::DateTools::Resolution;
using QueryParserTestBase =
    org::apache::lucene::queryparser::util::QueryParserTestBase;
using Query = org::apache::lucene::search::Query;

/**
 * Tests QueryParser.
 */
class TestStandardQP : public QueryParserTestBase
{
  GET_CLASS_NAME(TestStandardQP)

public:
  virtual std::shared_ptr<StandardQueryParser>
  getParser(std::shared_ptr<Analyzer> a) ;

  virtual std::shared_ptr<Query>
  parse(const std::wstring &query,
        std::shared_ptr<StandardQueryParser> qp) ;

  std::shared_ptr<CommonQueryParserConfiguration> getParserConfig(
      std::shared_ptr<Analyzer> a)  override;

  std::shared_ptr<Query>
  getQuery(const std::wstring &query,
           std::shared_ptr<CommonQueryParserConfiguration>
               cqpC)  override;

  std::shared_ptr<Query>
  getQuery(const std::wstring &query,
           std::shared_ptr<Analyzer> a)  override;

  bool isQueryParserException(std::runtime_error exception) override;

  void setDefaultOperatorOR(
      std::shared_ptr<CommonQueryParserConfiguration> cqpC) override;

  void setDefaultOperatorAND(
      std::shared_ptr<CommonQueryParserConfiguration> cqpC) override;

  void setAutoGeneratePhraseQueries(
      std::shared_ptr<CommonQueryParserConfiguration> cqpC,
      bool value) override;

  void setDateResolution(std::shared_ptr<CommonQueryParserConfiguration> cqpC,
                         std::shared_ptr<std::wstring> field,
                         Resolution value) override;

  void testOperatorVsWhitespace()  override;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestStandardQP> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(std::shared_ptr<TestStandardQP> outerInstance);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  void testRangeWithPhrase()  override;

  void testEscapedVsQuestionMarkAsWildcard()  override;

  void testEscapedWildcard()  override;

  void testAutoGeneratePhraseQueriesOn()  override;

  void testStarParsing()  override;

  void testDefaultOperator()  override;

  void testNewFieldQuery()  override;

  // TODO: Remove this specialization once the flexible standard parser gets
  // multi-word synonym support
  void testQPA()  override;

protected:
  std::shared_ptr<TestStandardQP> shared_from_this()
  {
    return std::static_pointer_cast<TestStandardQP>(
        org.apache.lucene.queryparser.util
            .QueryParserTestBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::standard
