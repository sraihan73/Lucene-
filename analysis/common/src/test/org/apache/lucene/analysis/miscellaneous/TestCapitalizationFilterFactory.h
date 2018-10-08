#pragma once
#include "../util/BaseTokenStreamFactoryTestCase.h"
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
namespace org::apache::lucene::analysis::miscellaneous
{

using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

class TestCapitalizationFilterFactory : public BaseTokenStreamFactoryTestCase
{
  GET_CLASS_NAME(TestCapitalizationFilterFactory)

public:
  virtual void testCapitalization() ;

  virtual void testCapitalization2() ;

  /** first is forced, but it's not a keep word, either */
  virtual void testCapitalization3() ;

  virtual void testCapitalization4() ;

  virtual void testCapitalization5() ;

  virtual void testCapitalization6() ;

  virtual void testCapitalization7() ;

  virtual void testCapitalization8() ;

  virtual void testCapitalization9() ;

  virtual void testCapitalization10() ;

  /** using "McK" as okPrefix */
  virtual void testCapitalization11() ;

  /** test with numbers */
  virtual void testCapitalization12() ;

  virtual void testCapitalization13() ;

  virtual void testKeepIgnoreCase() ;

  virtual void testKeepIgnoreCase2() ;

  virtual void testKeepIgnoreCase3() ;

  /**
   * Test CapitalizationFilterFactory's minWordLength option.
   *
   * This is very weird when combined with ONLY_FIRST_WORD!!!
   */
  virtual void testMinWordLength() ;

  /**
   * Test CapitalizationFilterFactory's maxWordCount option with only words of 1
   * in each token (it should do nothing)
   */
  virtual void testMaxWordCount() ;

  /**
   * Test CapitalizationFilterFactory's maxWordCount option when exceeded
   */
  virtual void testMaxWordCount2() ;

  /**
   * Test CapitalizationFilterFactory's maxTokenLength option when exceeded
   *
   * This is weird, it is not really a max, but inclusive (look at 'is')
   */
  virtual void testMaxTokenLength() ;

  /**
   * Test CapitalizationFilterFactory's forceFirstLetter option
   */
  virtual void testForceFirstLetterWithKeep() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

  /**
   * Test that invalid arguments result in exception
   */
  virtual void testInvalidArguments() ;

protected:
  std::shared_ptr<TestCapitalizationFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestCapitalizationFilterFactory>(
        org.apache.lucene.analysis.util
            .BaseTokenStreamFactoryTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
