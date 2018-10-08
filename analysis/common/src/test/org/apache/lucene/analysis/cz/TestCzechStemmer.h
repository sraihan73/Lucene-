#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::analysis::cz
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Test the Czech Stemmer.
 *
 * Note: it's algorithmic, so some stems are nonsense
 *
 */
class TestCzechStemmer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestCzechStemmer)

  /**
   * Test showing how masculine noun forms conflate
   */
public:
  virtual void testMasculineNouns() ;

  /**
   * Test showing how feminine noun forms conflate
   */
  virtual void testFeminineNouns() ;

  /**
   * Test showing how neuter noun forms conflate
   */
  virtual void testNeuterNouns() ;

  /**
   * Test showing how adjectival forms conflate
   */
  virtual void testAdjectives() ;

  /**
   * Test some possessive suffixes
   */
  virtual void testPossessive() ;

  /**
   * Test some exceptional rules, implemented as rewrites.
   */
  virtual void testExceptions() ;

  /**
   * Test that very short words are not stemmed.
   */
  virtual void testDontStem() ;

  virtual void testWithKeywordAttribute() ;

  virtual void testEmptyTerm() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestCzechStemmer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestCzechStemmer> outerInstance);

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

protected:
  std::shared_ptr<TestCzechStemmer> shared_from_this()
  {
    return std::static_pointer_cast<TestCzechStemmer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::cz
