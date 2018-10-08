#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringbuilder.h"
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
namespace org::apache::lucene::analysis::standard
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class TestUAX29URLEmailAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestUAX29URLEmailAnalyzer)

private:
  std::shared_ptr<Analyzer> a;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testHugeDoc() ;

  virtual void testArmenian() ;

  virtual void testAmharic() ;

  virtual void testArabic() ;

  virtual void testAramaic() ;

  virtual void testBengali() ;

  virtual void testFarsi() ;

  virtual void testGreek() ;

  virtual void testThai() ;

  virtual void testLao() ;

  virtual void testTibetan() ;

  /*
   * For chinese, tokenize as char (these can later form bigrams or whatever)
   */
  virtual void testChinese() ;

  virtual void testEmpty() ;

  /* test various jira issues this analyzer is related to */

  virtual void testLUCENE1545() ;

  /* Tests from StandardAnalyzer, just to show behavior is similar */
  virtual void testAlphanumericSA() ;

  virtual void testDelimitersSA() ;

  virtual void testApostrophesSA() ;

  virtual void testNumericSA() ;

  virtual void testTextWithNumbersSA() ;

  virtual void testVariousTextSA() ;

  virtual void testKoreanSA() ;

  virtual void testOffsets() ;

  virtual void testTypes() ;

  virtual void testSupplementary() ;

  virtual void testKorean() ;

  virtual void testJapanese() ;

  virtual void testCombiningMarks() ;

  virtual void testBasicEmails() ;

  virtual void testMailtoSchemeEmails() ;

  virtual void testBasicURLs() ;

  virtual void testNoSchemeURLs() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

  virtual void testMaxTokenLengthDefault() ;

  virtual void testMaxTokenLengthNonDefault() ;

protected:
  std::shared_ptr<TestUAX29URLEmailAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestUAX29URLEmailAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::standard
