#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringbuilder.h"
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
namespace org::apache::lucene::analysis::charfilter
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class HTMLStripCharFilterTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(HTMLStripCharFilterTest)

private:
  static std::shared_ptr<Analyzer> newTestAnalyzer();

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  public:
    AnalyzerAnonymousInnerClass();

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

  // this is some text  here is a  link  and another  link . This is an entity:
  // & plus a <.  Here is an &
  //
public:
  virtual void test() ;

  // Some sanity checks, but not a full-fledged check
  virtual void testHTML() ;

  virtual void testMSWord14GeneratedHTML() ;

  virtual void testGamma() ;

  virtual void testEntities() ;

  virtual void testMoreEntities() ;

  virtual void testReserved() ;

  virtual void testMalformedHTML() ;

  virtual void testBufferOverflow() ;

private:
  void appendChars(std::shared_ptr<StringBuilder> testBuilder, int numChars);

public:
  virtual void testComment() ;

  virtual void doTestOffsets(const std::wstring &in_) ;

  virtual void testOffsets() ;

  static void
  assertLegalOffsets(const std::wstring &in_) ;

  virtual void testLegalOffsets() ;

  virtual void testRandom() ;

  virtual void testRandomHugeStrings() ;

  virtual void testCloseBR() ;

  virtual void testServerSideIncludes() ;

  virtual void testScriptQuotes() ;

  virtual void testEscapeScript() ;

  virtual void testStyle() ;

  virtual void testEscapeStyle() ;

  virtual void testBR() ;
  virtual void testEscapeBR() ;

  virtual void testInlineTagsNoSpace() ;

  virtual void testCDATA() ;

  virtual void testUnclosedAngleBang() ;

  virtual void testUppercaseCharacterEntityVariants() ;

  virtual void
  testMSWordMalformedProcessingInstruction() ;

  virtual void testSupplementaryCharsInTags() ;

  virtual void testRandomBrokenHTML() ;

  virtual void testRandomText() ;

  virtual void testUTF16Surrogates() ;

  static void assertHTMLStripsTo(
      const std::wstring &input, const std::wstring &gold,
      std::shared_ptr<Set<std::wstring>> escapedTags) ;

  static void assertHTMLStripsTo(
      std::shared_ptr<Reader> input, const std::wstring &gold,
      std::shared_ptr<Set<std::wstring>> escapedTags) ;

protected:
  std::shared_ptr<HTMLStripCharFilterTest> shared_from_this()
  {
    return std::static_pointer_cast<HTMLStripCharFilterTest>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::charfilter
