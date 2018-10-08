#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
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
namespace org::apache::lucene::analysis::nl
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Test the Dutch Stem Filter, which only modifies the term text.
 *
 * The code states that it uses the snowball algorithm, but tests reveal some
 * differences.
 *
 */
class TestDutchAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestDutchAnalyzer)

public:
  virtual void testWithSnowballExamples() ;

  virtual void testSnowballCorrectness() ;

  virtual void testReusableTokenStream() ;

  virtual void testExclusionTableViaCtor() ;

  /**
   * check that the default stem overrides are used
   * even if you use a non-default ctor.
   */
  virtual void testStemOverrides() ;

  virtual void testEmptyStemDictionary() ;

  /**
   * Test that stopwords are not case sensitive
   */
  virtual void testStopwordsCasing() ;

private:
  void check(const std::wstring &input,
             const std::wstring &expected) ;

  /** blast some random strings through the analyzer */
public:
  virtual void testRandomStrings() ;

protected:
  std::shared_ptr<TestDutchAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestDutchAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::nl
