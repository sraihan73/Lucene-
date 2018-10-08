#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "BaseSynonymParserTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/synonym/SynonymMap.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"

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
namespace org::apache::lucene::analysis::synonym
{

/**
 * Tests parser for the Solr synonyms format
 * @lucene.experimental
 */
class TestSolrSynonymParser : public BaseSynonymParserTestCase
{
  GET_CLASS_NAME(TestSolrSynonymParser)

  /** Tests some simple examples from the solr wiki */
public:
  virtual void testSimple() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestSolrSynonymParser> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestSolrSynonymParser> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

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

  /** parse a syn file with bad syntax */
public:
  virtual void testInvalidDoubleMap() ;

  /** parse a syn file with bad syntax */
  virtual void testInvalidAnalyzesToNothingOutput() ;

  /** parse a syn file with bad syntax */
  virtual void testInvalidAnalyzesToNothingInput() ;

  /** parse a syn file with bad syntax */
  virtual void testInvalidPositionsInput() ;

  /** parse a syn file with bad syntax */
  virtual void testInvalidPositionsOutput() ;

  /** parse a syn file with some escaped syntax chars */
  virtual void testEscapedStuff() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestSolrSynonymParser> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestSolrSynonymParser> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

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

  /** Verify type of token and positionLength after analyzer. */
public:
  virtual void testPositionLengthAndTypeSimple() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestSolrSynonymParser> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<TestSolrSynonymParser> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  /** Test parsing of simple examples. */
public:
  virtual void testParseSimple() ;

protected:
  std::shared_ptr<TestSolrSynonymParser> shared_from_this()
  {
    return std::static_pointer_cast<TestSolrSynonymParser>(
        BaseSynonymParserTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/synonym/
