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
namespace org::apache::lucene::analysis::de
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class TestGermanAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestGermanAnalyzer)
public:
  virtual void testReusableTokenStream() ;

  virtual void testWithKeywordAttribute() ;

  virtual void testStemExclusionTable() ;

  /** test some features of the new snowball filter
   * these only pass with LATEST, not if you use o.a.l.a.de.GermanStemmer
   */
  virtual void testGermanSpecials() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

protected:
  std::shared_ptr<TestGermanAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestGermanAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::de
