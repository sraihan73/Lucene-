#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"
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
namespace org::apache::lucene::analysis::shingle
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Directory = org::apache::lucene::store::Directory;

/**
 * A test class for ShingleAnalyzerWrapper as regards queries and scoring.
 */
class ShingleAnalyzerWrapperTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(ShingleAnalyzerWrapperTest)
private:
  std::shared_ptr<Analyzer> analyzer;
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<Directory> directory;

  /**
   * Set up a new index in RAM with three test phrases and the supplied
   * Analyzer.
   *
   * @throws Exception if an error occurs with index writer or searcher
   */
public:
  void setUp()  override;

  void tearDown()  override;

protected:
  virtual void compareRanks(std::deque<std::shared_ptr<ScoreDoc>> &hits,
                            std::deque<int> &ranks) ;

  /*
   * This shows how to construct a phrase query containing shingles.
   */
public:
  virtual void
  testShingleAnalyzerWrapperPhraseQuery() ;

  /*
   * How to construct a bool query with shingles. A query like this will
   * implicitly score those documents higher that contain the words in the query
   * in the right order and adjacent to each other.
   */
  virtual void
  testShingleAnalyzerWrapperBooleanQuery() ;

  virtual void testReusableTokenStream() ;

  virtual void testNonDefaultMinShingleSize() ;

  virtual void
  testNonDefaultMinAndSameMaxShingleSize() ;

  virtual void testNoTokenSeparator() ;

  virtual void testNullTokenSeparator() ;

  virtual void testAltTokenSeparator() ;

  virtual void testAltFillerToken() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<ShingleAnalyzerWrapperTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<ShingleAnalyzerWrapperTest> outerInstance);

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
    std::shared_ptr<ShingleAnalyzerWrapperTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<ShingleAnalyzerWrapperTest> outerInstance);

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

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<ShingleAnalyzerWrapperTest> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<ShingleAnalyzerWrapperTest> outerInstance);

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

public:
  virtual void
  testOutputUnigramsIfNoShinglesSingleToken() ;

protected:
  std::shared_ptr<ShingleAnalyzerWrapperTest> shared_from_this()
  {
    return std::static_pointer_cast<ShingleAnalyzerWrapperTest>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/shingle/
