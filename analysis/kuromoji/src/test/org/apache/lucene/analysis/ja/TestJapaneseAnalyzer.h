#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
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
namespace org::apache::lucene::analysis::ja
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Test Kuromoji Japanese morphological analyzer
 */
class TestJapaneseAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestJapaneseAnalyzer)
  /** This test fails with NPE when the
   * stopwords file is missing in classpath */
public:
  virtual void testResourcesAvailable();

  /**
   * An example sentence, test removal of particles, etc by POS,
   * lemmatization with the basic form, and that position increments
   * and offsets are correct.
   */
  virtual void testBasics() ;

  /**
   * Test that search mode is enabled and working by default
   */
  virtual void testDecomposition() ;

  /**
   * blast random strings against the analyzer
   */
  virtual void testRandom() ;

  /** blast some random large strings through the analyzer */
  virtual void testRandomHugeStrings() ;

  // Copied from TestJapaneseTokenizer, to make sure passing
  // user dict to analyzer works:
  virtual void testUserDict3() ;

  // LUCENE-3897: this string (found by running all jawiki
  // XML through JapaneseAnalyzer) caused AIOOBE
  virtual void testCuriousString() ;

  // LUCENE-3897: this string (found by
  // testHugeRandomStrings) tripped assert
  virtual void testAnotherCuriousString() ;

  // LUCENE-3897: this string (found by
  // testHugeRandomStrings) tripped assert
  virtual void testYetAnotherCuriousString() ;

  virtual void test4thCuriousString() ;

  virtual void test5thCuriousString() ;

protected:
  std::shared_ptr<TestJapaneseAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestJapaneseAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ja/
