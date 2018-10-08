#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
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
namespace org::apache::lucene::analysis
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestDelegatingAnalyzerWrapper : public LuceneTestCase
{
  GET_CLASS_NAME(TestDelegatingAnalyzerWrapper)

public:
  virtual void testDelegatesNormalization();

private:
  class DelegatingAnalyzerWrapperAnonymousInnerClass
      : public DelegatingAnalyzerWrapper
  {
    GET_CLASS_NAME(DelegatingAnalyzerWrapperAnonymousInnerClass)
  private:
    std::shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer1;

  public:
    DelegatingAnalyzerWrapperAnonymousInnerClass(
        std::shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance,
        std::shared_ptr<UnknownType> GLOBAL_REUSE_STRATEGY,
        std::shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer1);

  protected:
    std::shared_ptr<Analyzer>
    getWrappedAnalyzer(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<DelegatingAnalyzerWrapperAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DelegatingAnalyzerWrapperAnonymousInnerClass>(
          DelegatingAnalyzerWrapper::shared_from_this());
    }
  };

private:
  class DelegatingAnalyzerWrapperAnonymousInnerClass2
      : public DelegatingAnalyzerWrapper
  {
    GET_CLASS_NAME(DelegatingAnalyzerWrapperAnonymousInnerClass2)
  private:
    std::shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer2;

  public:
    DelegatingAnalyzerWrapperAnonymousInnerClass2(
        std::shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance,
        std::shared_ptr<UnknownType> GLOBAL_REUSE_STRATEGY,
        std::shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer2);

  protected:
    std::shared_ptr<Analyzer>
    getWrappedAnalyzer(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<DelegatingAnalyzerWrapperAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DelegatingAnalyzerWrapperAnonymousInnerClass2>(
          DelegatingAnalyzerWrapper::shared_from_this());
    }
  };

public:
  virtual void testDelegatesAttributeFactory() ;

private:
  class DelegatingAnalyzerWrapperAnonymousInnerClass3
      : public DelegatingAnalyzerWrapper
  {
    GET_CLASS_NAME(DelegatingAnalyzerWrapperAnonymousInnerClass3)
  private:
    std::shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer1;

  public:
    DelegatingAnalyzerWrapperAnonymousInnerClass3(
        std::shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance,
        std::shared_ptr<UnknownType> GLOBAL_REUSE_STRATEGY,
        std::shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer1);

  protected:
    std::shared_ptr<Analyzer>
    getWrappedAnalyzer(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<DelegatingAnalyzerWrapperAnonymousInnerClass3>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DelegatingAnalyzerWrapperAnonymousInnerClass3>(
          DelegatingAnalyzerWrapper::shared_from_this());
    }
  };

public:
  virtual void testDelegatesCharFilter() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance);

  protected:
    std::shared_ptr<Reader>
    initReaderForNormalization(const std::wstring &fieldName,
                               std::shared_ptr<Reader> reader) override;
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          Analyzer::shared_from_this());
    }
  };

private:
  class DelegatingAnalyzerWrapperAnonymousInnerClass4
      : public DelegatingAnalyzerWrapper
  {
    GET_CLASS_NAME(DelegatingAnalyzerWrapperAnonymousInnerClass4)
  private:
    std::shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer1;

  public:
    DelegatingAnalyzerWrapperAnonymousInnerClass4(
        std::shared_ptr<TestDelegatingAnalyzerWrapper> outerInstance,
        std::shared_ptr<UnknownType> GLOBAL_REUSE_STRATEGY,
        std::shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer1);

  protected:
    std::shared_ptr<Analyzer>
    getWrappedAnalyzer(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<DelegatingAnalyzerWrapperAnonymousInnerClass4>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DelegatingAnalyzerWrapperAnonymousInnerClass4>(
          DelegatingAnalyzerWrapper::shared_from_this());
    }
  };

private:
  class DummyCharFilter : public CharFilter
  {
    GET_CLASS_NAME(DummyCharFilter)

  private:
    const wchar_t match, repl;

  public:
    DummyCharFilter(std::shared_ptr<Reader> input, wchar_t match, wchar_t repl);

  protected:
    int correct(int currentOff) override;

  public:
    int read(std::deque<wchar_t> &cbuf, int off,
             int len)  override;

  protected:
    std::shared_ptr<DummyCharFilter> shared_from_this()
    {
      return std::static_pointer_cast<DummyCharFilter>(
          CharFilter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestDelegatingAnalyzerWrapper> shared_from_this()
  {
    return std::static_pointer_cast<TestDelegatingAnalyzerWrapper>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
