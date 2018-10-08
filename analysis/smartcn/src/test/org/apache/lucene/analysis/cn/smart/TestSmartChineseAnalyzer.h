#pragma once
#include "../../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::analysis::cn::smart
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class TestSmartChineseAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestSmartChineseAnalyzer)

public:
  virtual void testChineseStopWordsDefault() ;

  /*
   * This test is the same as the above, except with two phrases.
   * This tests to ensure the SentenceTokenizer->WordTokenFilter chain works
   * correctly.
   */
  virtual void
  testChineseStopWordsDefaultTwoPhrases() ;

  /*
   * This test is for test smartcn HHMMSegmenter should correctly handle
   * surrogate character.
   */
  virtual void testSurrogatePairCharacter() ;

  /*
   * This test is the same as the above, except using an ideographic space as a
   * separator. This tests to ensure the stopwords are working correctly.
   */
  virtual void
  testChineseStopWordsDefaultTwoPhrasesIdeoSpace() ;

  /*
   * Punctuation is handled in a strange way if you disable stopwords
   * In this example the IDEOGRAPHIC FULL STOP is converted into a comma.
   * if you don't supply (true) to the constructor, or use a different stopwords
   * deque, then punctuation is indexed.
   */
  virtual void testChineseStopWordsOff() ;

  /*
   * Check that position increments after stopwords are correct,
   * when stopfilter is configured with enablePositionIncrements
   */
  virtual void testChineseStopWords2() ;

  virtual void testChineseAnalyzer() ;

  /*
   * English words are lowercased and porter-stemmed.
   */
  virtual void testMixedLatinChinese() ;

  /*
   * Numerics are parsed as their own tokens
   */
  virtual void testNumerics() ;

  /*
   * Full width alphas and numerics are folded to half-width
   */
  virtual void testFullWidth() ;

  /*
   * Presentation form delimiters are removed
   */
  virtual void testDelimiters() ;

  /*
   * Text from writing systems other than Chinese and Latin are parsed as
   * individual characters. (regardless of Unicode category)
   */
  virtual void testNonChinese() ;

  /*
   * Test what the analyzer does with out-of-vocabulary words.
   * In this case the name is Yousaf Raza Gillani.
   * Currently it is being analyzed into single characters...
   */
  virtual void testOOV() ;

  virtual void testOffsets() ;

  virtual void testReusableTokenStream() ;

  // LUCENE-3026
  virtual void testLargeDocument() ;

  // LUCENE-3026
  virtual void testLargeSentence() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

  /** blast some random large strings through the analyzer */
  virtual void testRandomHugeStrings() ;

protected:
  std::shared_ptr<TestSmartChineseAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestSmartChineseAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::cn::smart
