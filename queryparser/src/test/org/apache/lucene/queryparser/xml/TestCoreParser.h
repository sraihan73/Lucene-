#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/queryparser/xml/CoreParser.h"
#include  "core/src/java/org/apache/lucene/queryparser/xml/CoreParserTestIndexData.h"
#include  "core/src/java/org/apache/lucene/queryparser/xml/ParserException.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

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
namespace org::apache::lucene::queryparser::xml
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestCoreParser : public LuceneTestCase
{
  GET_CLASS_NAME(TestCoreParser)

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  static const std::wstring defaultField_;

  // C++ NOTE: Fields cannot have the same name as methods:
  static std::shared_ptr<Analyzer> analyzer_;
  // C++ NOTE: Fields cannot have the same name as methods:
  static std::shared_ptr<CoreParser> coreParser_;

  // C++ NOTE: Fields cannot have the same name as methods:
  static std::shared_ptr<CoreParserTestIndexData> indexData_;

protected:
  virtual std::shared_ptr<Analyzer> newAnalyzer();

  virtual std::shared_ptr<CoreParser>
  newCoreParser(const std::wstring &defaultField,
                std::shared_ptr<Analyzer> analyzer);

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

  virtual void testTermQueryXML() ;

  virtual void test_DOCTYPE_TermQueryXML() ;

  virtual void test_ENTITY_TermQueryXML() ;

  virtual void testTermQueryEmptyXML() ;

  virtual void testTermsQueryXML() ;

  virtual void testBooleanQueryXML() ;

  virtual void testDisjunctionMaxQueryXML() ;

  virtual void testRangeQueryXML() ;

  virtual void testUserQueryXML() ;

  virtual void testCustomFieldUserQueryXML() throw(ParserException,
                                                   IOException);

  virtual void testBoostingTermQueryXML() ;

  virtual void testSpanTermXML() ;

  virtual void testConstantScoreQueryXML() ;

  virtual void testMatchAllDocsPlusFilterXML() throw(ParserException,
                                                     IOException);

  virtual void testNestedBooleanQuery() ;

  virtual void testPointRangeQuery() ;

  virtual void testPointRangeQueryWithoutLowerTerm() throw(ParserException,
                                                           IOException);

  virtual void testPointRangeQueryWithoutUpperTerm() throw(ParserException,
                                                           IOException);

  virtual void testPointRangeQueryWithoutRange() throw(ParserException,
                                                       IOException);

  //================= Helper methods ===================================

protected:
  virtual std::wstring defaultField();

  virtual std::shared_ptr<Analyzer> analyzer();

  virtual std::shared_ptr<CoreParser> coreParser();

private:
  std::shared_ptr<CoreParserTestIndexData> indexData();

protected:
  virtual std::shared_ptr<IndexReader> reader();

  virtual std::shared_ptr<IndexSearcher> searcher();

  virtual void parseShouldFail(
      const std::wstring &xmlFileName,
      const std::wstring &expectedParserExceptionMessage) ;

  virtual std::shared_ptr<Query>
  parse(const std::wstring &xmlFileName) ;

  virtual std::shared_ptr<SpanQuery>
  parseAsSpan(const std::wstring &xmlFileName) throw(ParserException,
                                                     IOException);

private:
  std::shared_ptr<Query> implParse(const std::wstring &xmlFileName,
                                   bool span) throw(ParserException,
                                                    IOException);

protected:
  virtual std::shared_ptr<Query>
  rewrite(std::shared_ptr<Query> q) ;

  virtual void dumpResults(const std::wstring &qType, std::shared_ptr<Query> q,
                           int numDocs) ;

protected:
  std::shared_ptr<TestCoreParser> shared_from_this()
  {
    return std::static_pointer_cast<TestCoreParser>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/xml/
