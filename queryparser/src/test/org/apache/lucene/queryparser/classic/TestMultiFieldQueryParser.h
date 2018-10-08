#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/classic/ParseException.h"

#include  "core/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
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
namespace org::apache::lucene::queryparser::classic
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests QueryParser.
 */
class TestMultiFieldQueryParser : public LuceneTestCase
{
  GET_CLASS_NAME(TestMultiFieldQueryParser)

  /** test stop words parsing for both the non static form, and for the
   * corresponding static form (qtxt, fields[]). */
public:
  virtual void testStopwordsParsing() ;

  // verify parsing of query using a stopping analyzer
private:
  void assertStopQueryEquals(
      const std::wstring &qtxt,
      const std::wstring &expectedRes) ;

public:
  virtual void testSimple() ;

  virtual void testBoostsSimple() ;

  virtual void testStaticMethod1() ;

  virtual void testStaticMethod2() ;

  virtual void testStaticMethod2Old() ;

  virtual void testStaticMethod3() ;

  virtual void testStaticMethod3Old() ;

  virtual void testAnalyzerReturningNull() ;

  virtual void testStopWordSearching() ;

  /**
   * Return no tokens for field "f1".
   */
private:
  class AnalyzerReturningNull : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerReturningNull)
  public:
    std::shared_ptr<MockAnalyzer> stdAnalyzer =
        std::make_shared<MockAnalyzer>(LuceneTestCase::random());

    AnalyzerReturningNull();

  protected:
    std::shared_ptr<Reader> initReader(const std::wstring &fieldName,
                                       std::shared_ptr<Reader> reader) override;

  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerReturningNull> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerReturningNull>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testSimpleRegex() ;

  /** whitespace+lowercase analyzer with synonyms (dogs,dog) and (guinea
   * pig,cavy) */
private:
  class MockSynonymAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(MockSynonymAnalyzer)
  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<MockSynonymAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<MockSynonymAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testSynonyms() ;

protected:
  std::shared_ptr<TestMultiFieldQueryParser> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiFieldQueryParser>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/classic/
