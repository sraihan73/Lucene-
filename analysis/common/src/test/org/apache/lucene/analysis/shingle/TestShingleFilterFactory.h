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
namespace org::apache::lucene::analysis::shingle
{

using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

/**
 * Simple tests to ensure the Shingle filter factory works.
 */
class TestShingleFilterFactory : public BaseTokenStreamFactoryTestCase
{
  GET_CLASS_NAME(TestShingleFilterFactory)
  /**
   * Test the defaults
   */
public:
  virtual void testDefaults() ;

  /**
   * Test with unigrams disabled
   */
  virtual void testNoUnigrams() ;

  /**
   * Test with a higher max shingle size
   */
  virtual void testMaxShingleSize() ;

  /**
   * Test with higher min (and max) shingle size
   */
  virtual void testMinShingleSize() ;

  /**
   * Test with higher min (and max) shingle size and with unigrams disabled
   */
  virtual void testMinShingleSizeNoUnigrams() ;

  /**
   * Test with higher same min and max shingle size
   */
  virtual void testEqualMinAndMaxShingleSize() ;

  /**
   * Test with higher same min and max shingle size and with unigrams disabled
   */
  virtual void
  testEqualMinAndMaxShingleSizeNoUnigrams() ;

  /**
   * Test with a non-default token separator
   */
  virtual void testTokenSeparator() ;

  /**
   * Test with a non-default token separator and with unigrams disabled
   */
  virtual void testTokenSeparatorNoUnigrams() ;

  /**
   * Test with an empty token separator
   */
  virtual void testEmptyTokenSeparator() ;

  /**
   * Test with higher min (and max) shingle size
   * and with a non-default token separator
   */
  virtual void testMinShingleSizeAndTokenSeparator() ;

  /**
   * Test with higher min (and max) shingle size
   * and with a non-default token separator
   * and with unigrams disabled
   */
  virtual void
  testMinShingleSizeAndTokenSeparatorNoUnigrams() ;

  /**
   * Test with unigrams disabled except when there are no shingles, with
   * a single input token. Using default min/max shingle sizes: 2/2.  No
   * shingles will be created, since there are fewer input tokens than
   * min shingle size.  However, because outputUnigramsIfNoShingles is
   * set to true, even though outputUnigrams is set to false, one
   * unigram should be output.
   */
  virtual void testOutputUnigramsIfNoShingles() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

protected:
  std::shared_ptr<TestShingleFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestShingleFilterFactory>(
        org.apache.lucene.analysis.util
            .BaseTokenStreamFactoryTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::shingle
