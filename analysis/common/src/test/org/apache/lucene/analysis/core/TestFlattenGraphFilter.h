#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Token.h"

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

namespace org::apache::lucene::analysis::core
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Token = org::apache::lucene::analysis::Token;

class TestFlattenGraphFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestFlattenGraphFilter)

private:
  static std::shared_ptr<Token> token(const std::wstring &term, int posInc,
                                      int posLength, int startOffset,
                                      int endOffset);

public:
  virtual void testSimpleMock() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestFlattenGraphFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestFlattenGraphFilter> outerInstance);

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

  // Make sure graph is unchanged if it's already flat
public:
  virtual void testAlreadyFlatten() ;

  virtual void testWTF1() ;

  /** Same as testWTF1 except the "wtf" token comes out later */
  virtual void testWTF2() ;

  virtual void testNonGreedySynonyms() ;

  virtual void testNonGraph() ;

  virtual void testSimpleHole() ;

  virtual void testHoleUnderSyn() ;

  virtual void testStrangelyNumberedNodes() ;

  virtual void testTwoLongParallelPaths() ;

  // NOTE: TestSynonymGraphFilter's testRandomSyns also tests FlattenGraphFilter

protected:
  std::shared_ptr<TestFlattenGraphFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestFlattenGraphFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/core/
