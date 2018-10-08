#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "../../../../../../java/org/apache/lucene/analysis/morfologik/MorfologikAnalyzer.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
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
namespace org::apache::lucene::analysis::morfologik
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * TODO: The tests below rely on the order of returned lemmas, which is probably
 * not good.
 */
class TestMorfologikAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestMorfologikAnalyzer)

private:
  std::shared_ptr<Analyzer> getTestAnalyzer();

  /** Test stemming of single tokens with Morfologik library. */
public:
  void testSingleTokens() ;

  /** Test stemming of multiple tokens and proper term metrics. */
  void testMultipleTokens() ;

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") private void dumpTokens(std::wstring
  // input) throws java.io.IOException
  void dumpTokens(const std::wstring &input) ;

  /** Test reuse of MorfologikFilter with leftover stems. */
public:
  void testLeftoverStems() ;

  /** Test stemming of mixed-case tokens. */
  void testCase() ;

private:
  void assertPOSToken(std::shared_ptr<TokenStream> ts, const std::wstring &term,
                      std::deque<std::wstring> &tags) ;

  /** Test morphosyntactic annotations. */
public:
  void testPOSAttribute() ;

  /** */
  void testKeywordAttrTokens() ;

private:
  class MorfologikAnalyzerAnonymousInnerClass : public MorfologikAnalyzer
  {
    GET_CLASS_NAME(MorfologikAnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestMorfologikAnalyzer> outerInstance;

  public:
    MorfologikAnalyzerAnonymousInnerClass(
        std::shared_ptr<TestMorfologikAnalyzer> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &field) override;

  protected:
    std::shared_ptr<MorfologikAnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<MorfologikAnalyzerAnonymousInnerClass>(
          MorfologikAnalyzer::shared_from_this());
    }
  };

  /** blast some random strings through the analyzer */
public:
  virtual void testRandom() ;

protected:
  std::shared_ptr<TestMorfologikAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestMorfologikAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/morfologik/
