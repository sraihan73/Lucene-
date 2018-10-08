#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
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
namespace org::apache::lucene::analysis::phonetic
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/** Tests {@link BeiderMorseFilter} */
class TestBeiderMorseFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestBeiderMorseFilter)
private:
  std::shared_ptr<Analyzer> analyzer;

public:
  void setUp()  override;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestBeiderMorseFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestBeiderMorseFilter> outerInstance);

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

public:
  void tearDown()  override;

  /** generic, "exact" configuration */
  virtual void testBasicUsage() ;

  /** restrict the output to a set of possible origin languages */
  virtual void testLanguageSet() ;

private:
  class HashSetAnonymousInnerClass : public std::unordered_set<std::wstring>
  {
    GET_CLASS_NAME(HashSetAnonymousInnerClass)
  private:
    std::shared_ptr<TestBeiderMorseFilter> outerInstance;

  public:
    HashSetAnonymousInnerClass(
        std::shared_ptr<TestBeiderMorseFilter> outerInstance);

  protected:
    std::shared_ptr<HashSetAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HashSetAnonymousInnerClass>(
          java.util.HashSet<std::wstring>::shared_from_this());
    }
  };

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestBeiderMorseFilter> outerInstance;

    std::shared_ptr<LanguageSet> languages;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestBeiderMorseFilter> outerInstance,
        std::shared_ptr<LanguageSet> languages);

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

  /** for convenience, if the input yields no output, we pass it thru as-is */
public:
  virtual void testNumbers() ;

  virtual void testRandom() ;

  virtual void testEmptyTerm() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestBeiderMorseFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<TestBeiderMorseFilter> outerInstance);

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
  virtual void testCustomAttribute() ;

protected:
  std::shared_ptr<TestBeiderMorseFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestBeiderMorseFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::phonetic
