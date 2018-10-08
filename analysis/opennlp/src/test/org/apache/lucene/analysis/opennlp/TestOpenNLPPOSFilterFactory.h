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

namespace org::apache::lucene::analysis::opennlp
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Needs the OpenNLP Tokenizer because it creates full streams of punctuation.
 * The POS model is based on this tokenization.
 *
 * Tagging models are created from tiny test data in
 * opennlp/tools/test-model-data/ and are not very accurate.
 */
class TestOpenNLPPOSFilterFactory : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestOpenNLPPOSFilterFactory)

private:
  static const std::wstring SENTENCES;
  static std::deque<std::wstring> const SENTENCES_punc;
  static std::deque<int> const SENTENCES_startOffsets;
  static std::deque<int> const SENTENCES_endOffsets;
  static std::deque<std::wstring> const SENTENCES_posTags;

  static const std::wstring NO_BREAK;
  static std::deque<std::wstring> const NO_BREAK_terms;
  static std::deque<int> const NO_BREAK_startOffsets;
  static std::deque<int> const NO_BREAK_endOffsets;

  static const std::wstring sentenceModelFile;
  static const std::wstring tokenizerModelFile;
  static const std::wstring posTaggerModelFile;

  static std::deque<std::deque<char>>
  toPayloads(std::deque<std::wstring> &strings);

public:
  virtual void testBasic() ;

  virtual void testPOS() ;

  virtual void testNoBreak() ;

protected:
  std::shared_ptr<TestOpenNLPPOSFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestOpenNLPPOSFilterFactory>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::opennlp
