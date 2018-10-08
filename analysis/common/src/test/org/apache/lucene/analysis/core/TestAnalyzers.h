#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

class TestAnalyzers : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestAnalyzers)

public:
  virtual void testSimple() ;

  virtual void testNull() ;

  virtual void testStop() ;

  virtual void
  verifyPayload(std::shared_ptr<TokenStream> ts) ;

  // Make sure old style next() calls result in a new copy of payloads
  virtual void testPayloadCopy() ;

  // LUCENE-1150: Just a compile time test, to ensure the
  // StandardAnalyzer constants remain publicly accessible
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") public void
  // _testStandardConstants()
  virtual void _testStandardConstants();

private:
  class LowerCaseWhitespaceAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(LowerCaseWhitespaceAnalyzer)

  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<LowerCaseWhitespaceAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<LowerCaseWhitespaceAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class UpperCaseWhitespaceAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(UpperCaseWhitespaceAnalyzer)

  public:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<UpperCaseWhitespaceAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<UpperCaseWhitespaceAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  /**
   * Test that LowercaseFilter handles entire unicode range correctly
   */
public:
  virtual void testLowerCaseFilter() ;

  /**
   * Test that LowercaseFilter handles entire unicode range correctly
   */
  virtual void testUpperCaseFilter() ;

  /**
   * Test that LowercaseFilter handles the lowercasing correctly if the term
   * buffer has a trailing surrogate character leftover and the current term in
   * the buffer ends with a corresponding leading surrogate.
   */
  virtual void testLowerCaseFilterLowSurrogateLeftover() ;

  virtual void testLowerCaseTokenizer() ;

  virtual void testWhitespaceTokenizer() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

  /** blast some random large strings through the analyzer */
  virtual void testRandomHugeStrings() ;

protected:
  std::shared_ptr<TestAnalyzers> shared_from_this()
  {
    return std::static_pointer_cast<TestAnalyzers>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

class PayloadSetter final : public TokenFilter
{
  GET_CLASS_NAME(PayloadSetter)
public:
  std::shared_ptr<PayloadAttribute> payloadAtt;
  PayloadSetter(std::shared_ptr<TokenStream> input);

  std::deque<char> data = std::deque<char>(1);
  std::shared_ptr<BytesRef> p = std::make_shared<BytesRef>(data, 0, 1);

  bool incrementToken()  override;

protected:
  std::shared_ptr<PayloadSetter> shared_from_this()
  {
    return std::static_pointer_cast<PayloadSetter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/core/
