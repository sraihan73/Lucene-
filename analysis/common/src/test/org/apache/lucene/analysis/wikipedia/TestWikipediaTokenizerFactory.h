#pragma once
#include "../util/BaseTokenStreamFactoryTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>
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
namespace org::apache::lucene::analysis::wikipedia
{

using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

/**
 * Simple tests to ensure the wikipedia tokenizer is working.
 */
class TestWikipediaTokenizerFactory : public BaseTokenStreamFactoryTestCase
{
  GET_CLASS_NAME(TestWikipediaTokenizerFactory)

private:
  const std::wstring WIKIPEDIA = L"Wikipedia";
  const std::wstring TOKEN_OUTPUT = L"tokenOutput";
  const std::wstring UNTOKENIZED_TYPES = L"untokenizedTypes";

public:
  virtual void testTokenizer() ;

  virtual void testTokenizerTokensOnly() ;

  virtual void testTokenizerUntokenizedOnly() ;

  virtual void testTokenizerBoth() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

  virtual void testIllegalArguments() ;

protected:
  std::shared_ptr<TestWikipediaTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestWikipediaTokenizerFactory>(
        org.apache.lucene.analysis.util
            .BaseTokenStreamFactoryTestCase::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/analysis/wikipedia/
