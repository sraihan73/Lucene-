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
 * Needs the OpenNLP POS tagger for the POS tags.
 *
 * Tagging models are created from tiny test data in
 * opennlp/tools/test-model-data/ and are not very accurate.
 */
class TestOpenNLPChunkerFilterFactory : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestOpenNLPChunkerFilterFactory)

private:
  static const std::wstring SENTENCES;
  static std::deque<std::wstring> const SENTENCES_punc;
  static std::deque<int> const SENTENCES_startOffsets;
  static std::deque<int> const SENTENCES_endOffsets;
  static std::deque<std::wstring> const SENTENCES_chunks;

  static const std::wstring sentenceModelFile;
  static const std::wstring tokenizerModelFile;
  static const std::wstring posTaggerModelFile;
  static const std::wstring chunkerModelFile;

  static std::deque<std::deque<char>>
  toPayloads(std::deque<std::wstring> &strings);

public:
  virtual void testBasic() ;

  virtual void testPayloads() ;

protected:
  std::shared_ptr<TestOpenNLPChunkerFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestOpenNLPChunkerFilterFactory>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/opennlp/
