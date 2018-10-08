#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class CharacterRunAutomaton;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::analysis::core
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

/**
 * Compares MockTokenizer (which is simple with no optimizations) with
 * equivalent core tokenizers (that have optimizations like buffering).
 *
 * Any tests here need to probably consider unicode version of the JRE (it could
 * cause false fails).
 */
class TestDuelingAnalyzers : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestDuelingAnalyzers)
private:
  std::shared_ptr<CharacterRunAutomaton> jvmLetter;

public:
  void setUp()  override;

  virtual void testLetterAscii() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestDuelingAnalyzers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestDuelingAnalyzers> outerInstance);

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

  // not so useful since it's all one token?!
public:
  virtual void testLetterAsciiHuge() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestDuelingAnalyzers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestDuelingAnalyzers> outerInstance);

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

public:
  virtual void testLetterHtmlish() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestDuelingAnalyzers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<TestDuelingAnalyzers> outerInstance);

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
  virtual void testLetterHtmlishHuge() ;

private:
  class AnalyzerAnonymousInnerClass4 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass4)
  private:
    std::shared_ptr<TestDuelingAnalyzers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass4(
        std::shared_ptr<TestDuelingAnalyzers> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass4>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testLetterUnicode() ;

private:
  class AnalyzerAnonymousInnerClass5 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass5)
  private:
    std::shared_ptr<TestDuelingAnalyzers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass5(
        std::shared_ptr<TestDuelingAnalyzers> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass5>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testLetterUnicodeHuge() ;

private:
  class AnalyzerAnonymousInnerClass6 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass6)
  private:
    std::shared_ptr<TestDuelingAnalyzers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass6(
        std::shared_ptr<TestDuelingAnalyzers> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass6> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass6>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  // we only check a few core attributes here.
  // TODO: test other things
public:
  virtual void
  assertEquals(const std::wstring &s, std::shared_ptr<TokenStream> left,
               std::shared_ptr<TokenStream> right) ;

  // TODO: maybe push this out to _TestUtil or LuceneTestCase and always use it
  // instead?
private:
  static std::shared_ptr<Reader> newStringReader(const std::wstring &s);

protected:
  std::shared_ptr<TestDuelingAnalyzers> shared_from_this()
  {
    return std::static_pointer_cast<TestDuelingAnalyzers>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::core
