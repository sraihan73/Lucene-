#pragma once
#include "../util/BaseTokenStreamFactoryTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::util
{
class TokenFilterFactory;
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
namespace org::apache::lucene::analysis::synonym
{

using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

class TestSynonymFilterFactory : public BaseTokenStreamFactoryTestCase
{
  GET_CLASS_NAME(TestSynonymFilterFactory)

  /** checks for synonyms of "GB" in synonyms.txt */
private:
  void checkSolrSynonyms(std::shared_ptr<TokenFilterFactory> factory) throw(
      std::runtime_error);

  /** checks for synonyms of "second" in synonyms-wordnet.txt */
  void checkWordnetSynonyms(std::shared_ptr<TokenFilterFactory> factory) throw(
      std::runtime_error);

  /** test that we can parse and use the solr syn file */
public:
  virtual void testSynonyms() ;

  /** if the synonyms are completely empty, test that we still analyze correctly
   */
  virtual void testEmptySynonyms() ;

  virtual void testFormat() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

  /** Test that analyzer and tokenizerFactory is both specified */
  virtual void testAnalyzer() ;

  static const std::wstring TOK_SYN_ARG_VAL;
  static const std::wstring TOK_FOO_ARG_VAL;

  /** Test that we can parse TokenierFactory's arguments */
  virtual void testTokenizerFactoryArguments() ;

protected:
  std::shared_ptr<TestSynonymFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestSynonymFilterFactory>(
        org.apache.lucene.analysis.util
            .BaseTokenStreamFactoryTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::synonym
