#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/ReuseStrategy.h"

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
namespace org::apache::lucene::analysis
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

class TestMockAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestMockAnalyzer)

  /** Test a configuration that behaves a lot like WhitespaceAnalyzer */
public:
  virtual void testWhitespace() ;

  /** Test a configuration that behaves a lot like SimpleAnalyzer */
  virtual void testSimple() ;

  /** Test a configuration that behaves a lot like KeywordAnalyzer */
  virtual void testKeyword() ;

  // Test some regular expressions as tokenization patterns
  /** Test a configuration where each character is a term */
  virtual void testSingleChar() ;

  /** Test a configuration where two characters makes a term */
  virtual void testTwoChars() ;

  /** Test a configuration where three characters makes a term */
  virtual void testThreeChars() ;

  /** Test a configuration where word starts with one uppercase */
  virtual void testUppercase() ;

  /** Test a configuration that behaves a lot like StopAnalyzer */
  virtual void testStop() ;

  /** Test a configuration that behaves a lot like KeepWordFilter */
  virtual void testKeep() ;

  /** Test a configuration that behaves a lot like LengthFilter */
  virtual void testLength() ;

  /** Test MockTokenizer encountering a too long token */
  virtual void testTooLongToken() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestMockAnalyzer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestMockAnalyzer> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testLUCENE_3042() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

  /** blast some random strings through differently configured tokenizers */
  virtual void testRandomRegexps() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestMockAnalyzer> outerInstance;

    std::shared_ptr<CharacterRunAutomaton> dfa;
    bool lowercase = false;
    int limit = 0;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestMockAnalyzer> outerInstance,
        std::shared_ptr<CharacterRunAutomaton> dfa, bool lowercase, int limit);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testForwardOffsets() ;

  virtual void testWrapReader() ;

private:
  class AnalyzerWrapperAnonymousInnerClass : public AnalyzerWrapper
  {
    GET_CLASS_NAME(AnalyzerWrapperAnonymousInnerClass)
  private:
    std::shared_ptr<TestMockAnalyzer> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::Analyzer> delegate_;

  public:
    AnalyzerWrapperAnonymousInnerClass(
        std::shared_ptr<TestMockAnalyzer> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::Analyzer::ReuseStrategy>
            getReuseStrategy,
        std::shared_ptr<org::apache::lucene::analysis::Analyzer> delegate_);

  protected:
    std::shared_ptr<Reader> wrapReader(const std::wstring &fieldName,
                                       std::shared_ptr<Reader> reader) override;

    std::shared_ptr<Analyzer>
    getWrappedAnalyzer(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerWrapperAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerWrapperAnonymousInnerClass>(
          AnalyzerWrapper::shared_from_this());
    }
  };

public:
  virtual void testChangeGaps() ;

private:
  class DelegatingAnalyzerWrapperAnonymousInnerClass
      : public DelegatingAnalyzerWrapper
  {
    GET_CLASS_NAME(DelegatingAnalyzerWrapperAnonymousInnerClass)
  private:
    std::shared_ptr<TestMockAnalyzer> outerInstance;

    int positionGap = 0;
    int offsetGap = 0;
    std::shared_ptr<org::apache::lucene::analysis::Analyzer> delegate_;

  public:
    DelegatingAnalyzerWrapperAnonymousInnerClass(
        std::shared_ptr<TestMockAnalyzer> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::Analyzer::ReuseStrategy>
            getReuseStrategy,
        int positionGap, int offsetGap,
        std::shared_ptr<org::apache::lucene::analysis::Analyzer> delegate_);

  protected:
    std::shared_ptr<Analyzer>
    getWrappedAnalyzer(const std::wstring &fieldName) override;

  public:
    int getPositionIncrementGap(const std::wstring &fieldName) override;
    int getOffsetGap(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<DelegatingAnalyzerWrapperAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DelegatingAnalyzerWrapperAnonymousInnerClass>(
          DelegatingAnalyzerWrapper::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestMockAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestMockAnalyzer>(
        BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
