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
namespace org::apache::lucene::analysis::pattern
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class TestPatternCaptureGroupTokenFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestPatternCaptureGroupTokenFilter)

public:
  virtual void testNoPattern() ;

  virtual void testNoMatch() ;

  virtual void testNoCapture() ;

  virtual void testEmptyCapture() ;

  virtual void testCaptureAll() ;

  virtual void testCaptureStart() ;

  virtual void testCaptureMiddle() ;

  virtual void testCaptureEnd() ;

  virtual void testCaptureStartMiddle() ;

  virtual void testCaptureStartEnd() ;

  virtual void testCaptureMiddleEnd() ;

  virtual void testMultiCaptureOverlap() ;

  virtual void testMultiPattern() ;

  virtual void testCamelCase() ;

  virtual void testRandomString() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestPatternCaptureGroupTokenFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestPatternCaptureGroupTokenFilter> outerInstance);

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
  void testPatterns(const std::wstring &input,
                    std::deque<std::wstring> &regexes,
                    std::deque<std::wstring> &tokens,
                    std::deque<int> &startOffsets,
                    std::deque<int> &endOffsets, std::deque<int> &positions,
                    bool preserveOriginal) ;

protected:
  std::shared_ptr<TestPatternCaptureGroupTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestPatternCaptureGroupTokenFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/pattern/
