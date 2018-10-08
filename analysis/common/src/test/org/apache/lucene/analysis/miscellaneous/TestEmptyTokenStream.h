#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

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
namespace org::apache::lucene::analysis::miscellaneous
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class TestEmptyTokenStream : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestEmptyTokenStream)

public:
  virtual void testConsume() ;

private:
  class AnalyzerAnonymousInnerClass
      : public org::apache::lucene::analysis::Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<MissingClass> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::CharArraySet> protectedTerms;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<MissingClass> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::CharArraySet>
            protectedTerms);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class AnalyzerAnonymousInnerClass2
      : public org::apache::lucene::analysis::Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<MissingClass> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::CharArraySet> protectedTerms;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<MissingClass> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::CharArraySet>
            protectedTerms);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class AnalyzerAnonymousInnerClass3
      : public org::apache::lucene::analysis::Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<MissingClass> outerInstance;

    int64_t seed = 0;

  public:
    AnalyzerAnonymousInnerClass3(std::shared_ptr<MissingClass> outerInstance,
                                 int64_t seed);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testConsume2() ;

  virtual void testIndexWriter_LUCENE4656() ;

protected:
  std::shared_ptr<TestEmptyTokenStream> shared_from_this()
  {
    return std::static_pointer_cast<TestEmptyTokenStream>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
