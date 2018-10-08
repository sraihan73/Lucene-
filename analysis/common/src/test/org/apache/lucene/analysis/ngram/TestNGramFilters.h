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
namespace org::apache::lucene::analysis::ngram
{

using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

/**
 * Simple tests to ensure the NGram filter factories are working.
 */
class TestNGramFilters : public BaseTokenStreamFactoryTestCase
{
  GET_CLASS_NAME(TestNGramFilters)
  /**
   * Test NGramTokenizerFactory
   */
public:
  virtual void testNGramTokenizer() ;

  /**
   * Test NGramTokenizerFactory with min and max gram options
   */
  virtual void testNGramTokenizer2() ;

  /**
   * Test the NGramFilterFactory with old defaults
   */
  virtual void testNGramFilter() ;

  /**
   * Test the NGramFilterFactory with min and max gram options
   */
  virtual void testNGramFilter2() ;

  /**
   * Test NGramFilterFactory on tokens with payloads
   */
  virtual void testNGramFilterPayload() ;

  /**
   * Test EdgeNGramTokenizerFactory
   */
  virtual void testEdgeNGramTokenizer() ;

  /**
   * Test EdgeNGramTokenizerFactory with min and max gram size
   */
  virtual void testEdgeNGramTokenizer2() ;

  /**
   * Test EdgeNGramFilterFactory with old defaults
   */
  virtual void testEdgeNGramFilter() ;

  /**
   * Test EdgeNGramFilterFactory with min and max gram size
   */
  virtual void testEdgeNGramFilter2() ;

  /**
   * Test EdgeNGramFilterFactory on tokens with payloads
   */
  virtual void testEdgeNGramFilterPayload() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

protected:
  std::shared_ptr<TestNGramFilters> shared_from_this()
  {
    return std::static_pointer_cast<TestNGramFilters>(
        org.apache.lucene.analysis.util
            .BaseTokenStreamFactoryTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ngram/
