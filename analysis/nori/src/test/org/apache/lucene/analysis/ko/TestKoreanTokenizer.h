#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/POS.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/analysis/ko/dict/UserDictionary.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/analysis/ko/POS.h"

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
namespace org::apache::lucene::analysis::ko
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using UserDictionary = org::apache::lucene::analysis::ko::dict::UserDictionary;

class TestKoreanTokenizer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestKoreanTokenizer)
private:
  std::shared_ptr<Analyzer> analyzer, analyzerUnigram, analyzerDecompound,
      analyzerDecompoundKeep, analyzerReading;

public:
  static std::shared_ptr<UserDictionary> readDict();

  void setUp()  override;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestKoreanTokenizer> outerInstance;

    std::shared_ptr<UserDictionary> userDictionary;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestKoreanTokenizer> outerInstance,
        std::shared_ptr<UserDictionary> userDictionary);

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
    std::shared_ptr<TestKoreanTokenizer> outerInstance;

    std::shared_ptr<UserDictionary> userDictionary;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestKoreanTokenizer> outerInstance,
        std::shared_ptr<UserDictionary> userDictionary);

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
    std::shared_ptr<TestKoreanTokenizer> outerInstance;

    std::shared_ptr<UserDictionary> userDictionary;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<TestKoreanTokenizer> outerInstance,
        std::shared_ptr<UserDictionary> userDictionary);

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

private:
  class AnalyzerAnonymousInnerClass4 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass4)
  private:
    std::shared_ptr<TestKoreanTokenizer> outerInstance;

    std::shared_ptr<UserDictionary> userDictionary;

  public:
    AnalyzerAnonymousInnerClass4(
        std::shared_ptr<TestKoreanTokenizer> outerInstance,
        std::shared_ptr<UserDictionary> userDictionary);

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

private:
  class AnalyzerAnonymousInnerClass5 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass5)
  private:
    std::shared_ptr<TestKoreanTokenizer> outerInstance;

    std::shared_ptr<UserDictionary> userDictionary;

  public:
    AnalyzerAnonymousInnerClass5(
        std::shared_ptr<TestKoreanTokenizer> outerInstance,
        std::shared_ptr<UserDictionary> userDictionary);

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
  virtual void testSpaces() ;

  virtual void testPartOfSpeechs() ;

  virtual void testPartOfSpeechsWithCompound() ;

  virtual void testPartOfSpeechsWithInflects() ;

  virtual void testUnknownWord() ;

  virtual void testReading() ;

  virtual void testUserDict() ;

  /** blast some random strings through the tokenizer */
  virtual void testRandomStrings() ;

  /** blast some random large strings through the tokenizer */
  virtual void testRandomHugeStrings() ;

  virtual void testRandomHugeStringsMockGraphAfter() ;

private:
  class AnalyzerAnonymousInnerClass6 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass6)
  private:
    std::shared_ptr<TestKoreanTokenizer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass6(
        std::shared_ptr<TestKoreanTokenizer> outerInstance);

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

private:
  void assertReadings(std::shared_ptr<Analyzer> analyzer,
                      const std::wstring &input,
                      std::deque<std::wstring> &readings) ;

  void assertPartsOfSpeech(
      std::shared_ptr<Analyzer> analyzer, const std::wstring &input,
      std::deque<POS::Type> &posTypes, std::deque<POS::Tag> &leftPosTags,
      std::deque<POS::Tag> &rightPosTags) ;

protected:
  std::shared_ptr<TestKoreanTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<TestKoreanTokenizer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ko/
