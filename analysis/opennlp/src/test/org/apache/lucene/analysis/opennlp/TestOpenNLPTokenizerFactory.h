#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
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

namespace org::apache::lucene::analysis::opennlp
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Tests the Tokenizer as well- the Tokenizer needs the OpenNLP model files,
 * which this can load from src/test-files/opennlp/solr/conf
 *
 */
class TestOpenNLPTokenizerFactory : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestOpenNLPTokenizerFactory)

private:
  static std::wstring SENTENCES;
  static std::deque<std::wstring> SENTENCES_punc;
  static std::deque<int> SENTENCES_startOffsets;
  static std::deque<int> SENTENCES_endOffsets;

  static std::wstring SENTENCE1;
  static std::deque<std::wstring> SENTENCE1_punc;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTokenizer() throws java.io.IOException
  virtual void testTokenizer() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTokenizerNoSentenceDetector() throws
  // java.io.IOException
  virtual void testTokenizerNoSentenceDetector() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTokenizerNoTokenizer() throws
  // java.io.IOException
  virtual void testTokenizerNoTokenizer() ;

  // test analyzer caching the tokenizer
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testClose() throws java.io.IOException
  virtual void testClose() ;

private:
  class HashMapAnonymousInnerClass
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass)
  private:
    std::shared_ptr<TestOpenNLPTokenizerFactory> outerInstance;

  public:
    HashMapAnonymousInnerClass(
        std::shared_ptr<TestOpenNLPTokenizerFactory> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestOpenNLPTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestOpenNLPTokenizerFactory>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::opennlp
