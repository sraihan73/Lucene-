#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/AnalyzerWrapper.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/analysis/miscellaneous/PerFieldAnalyzerWrapper.h"
#include  "core/src/java/org/apache/lucene/analysis/ReuseStrategy.h"
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
namespace org::apache::lucene::analysis::miscellaneous
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class TestPerFieldAnalyzerWrapper : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestPerFieldAnalyzerWrapper)
public:
  virtual void testPerField() ;

  virtual void testReuseWrapped() ;

private:
  class AnalyzerWrapperAnonymousInnerClass : public AnalyzerWrapper
  {
    GET_CLASS_NAME(AnalyzerWrapperAnonymousInnerClass)
  private:
    std::shared_ptr<TestPerFieldAnalyzerWrapper> outerInstance;

    std::wstring text;
    std::shared_ptr<Analyzer> specialAnalyzer;
    std::shared_ptr<
        org::apache::lucene::analysis::miscellaneous::PerFieldAnalyzerWrapper>
        wrapper1;

  public:
    AnalyzerWrapperAnonymousInnerClass(
        std::shared_ptr<TestPerFieldAnalyzerWrapper> outerInstance,
        std::shared_ptr<Analyzer::ReuseStrategy> getReuseStrategy,
        const std::wstring &text, std::shared_ptr<Analyzer> specialAnalyzer,
        std::shared_ptr<org::apache::lucene::analysis::miscellaneous::
                            PerFieldAnalyzerWrapper>
            wrapper1);

  protected:
    std::shared_ptr<Analyzer>
    getWrappedAnalyzer(const std::wstring &fieldName) override;

    std::shared_ptr<Analyzer::TokenStreamComponents> wrapComponents(
        const std::wstring &fieldName,
        std::shared_ptr<Analyzer::TokenStreamComponents> components) override;

  protected:
    std::shared_ptr<AnalyzerWrapperAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerWrapperAnonymousInnerClass>(
          org.apache.lucene.analysis.AnalyzerWrapper::shared_from_this());
    }
  };

public:
  virtual void testCharFilters() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestPerFieldAnalyzerWrapper> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestPerFieldAnalyzerWrapper> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

    std::shared_ptr<Reader> initReader(const std::wstring &fieldName,
                                       std::shared_ptr<Reader> reader) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestPerFieldAnalyzerWrapper> shared_from_this()
  {
    return std::static_pointer_cast<TestPerFieldAnalyzerWrapper>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
