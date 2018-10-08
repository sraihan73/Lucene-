#pragma once
#include "../util/BaseTokenStreamFactoryTestCase.h"
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
namespace org::apache::lucene::analysis::standard
{

using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

/**
 * Simple tests to ensure the standard lucene factories are working.
 */
class TestStandardFactories : public BaseTokenStreamFactoryTestCase
{
  GET_CLASS_NAME(TestStandardFactories)
  /**
   * Test StandardTokenizerFactory
   */
public:
  virtual void testStandardTokenizer() ;

  virtual void testStandardTokenizerMaxTokenLength() ;

  /**
   * Test ClassicTokenizerFactory
   */
  virtual void testClassicTokenizer() ;

  virtual void testClassicTokenizerMaxTokenLength() ;

  /**
   * Test ClassicFilterFactory
   */
  virtual void testStandardFilter() ;

  /**
   * Test KeywordTokenizerFactory
   */
  virtual void testKeywordTokenizer() ;

  /**
   * Test WhitespaceTokenizerFactory
   */
  virtual void testWhitespaceTokenizer() ;

  /**
   * Test LetterTokenizerFactory
   */
  virtual void testLetterTokenizer() ;

  /**
   * Test LowerCaseTokenizerFactory
   */
  virtual void testLowerCaseTokenizer() ;

  /**
   * Ensure the ASCIIFoldingFilterFactory works
   */
  virtual void testASCIIFolding() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

protected:
  std::shared_ptr<TestStandardFactories> shared_from_this()
  {
    return std::static_pointer_cast<TestStandardFactories>(
        org.apache.lucene.analysis.util
            .BaseTokenStreamFactoryTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::standard
