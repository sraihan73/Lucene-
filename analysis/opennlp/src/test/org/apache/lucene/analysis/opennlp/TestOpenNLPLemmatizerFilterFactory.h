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

class TestOpenNLPLemmatizerFilterFactory : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestOpenNLPLemmatizerFilterFactory)

private:
  static const std::wstring SENTENCE;
  static std::deque<std::wstring> const SENTENCE_dict_punc;
  static std::deque<std::wstring> const SENTENCE_maxent_punc;
  static std::deque<std::wstring> const SENTENCE_posTags;

  static const std::wstring SENTENCES;
  static std::deque<std::wstring> const SENTENCES_dict_punc;
  static std::deque<std::wstring> const SENTENCES_maxent_punc;
  static std::deque<std::wstring> const SENTENCES_posTags;

  static const std::wstring SENTENCE_both;
  static std::deque<std::wstring> const SENTENCE_both_punc;
  static std::deque<std::wstring> const SENTENCE_both_posTags;

  static const std::wstring SENTENCES_both;
  static std::deque<std::wstring> const SENTENCES_both_punc;
  static std::deque<std::wstring> const SENTENCES_both_posTags;

  static std::deque<std::wstring> const SENTENCES_dict_keep_orig_punc;
  static std::deque<std::wstring> const SENTENCES_max_ent_keep_orig_punc;
  static std::deque<std::wstring> const SENTENCES_keep_orig_posTags;

  static std::deque<std::wstring> const SENTENCES_both_keep_orig_punc;
  static std::deque<std::wstring> const SENTENCES_both_keep_orig_posTags;

  static const std::wstring tokenizerModelFile;
  static const std::wstring sentenceModelFile;
  static const std::wstring posTaggerModelFile;
  static const std::wstring lemmatizerModelFile;
  static const std::wstring lemmatizerDictFile;

public:
  virtual void test1SentenceDictionaryOnly() ;

  virtual void test2SentencesDictionaryOnly() ;

  virtual void test1SentenceMaxEntOnly() ;

  virtual void test2SentencesMaxEntOnly() ;

  virtual void test1SentenceDictionaryAndMaxEnt() ;

  virtual void test2SentencesDictionaryAndMaxEnt() ;

  virtual void
  testKeywordAttributeAwarenessDictionaryOnly() ;

  virtual void
  testKeywordAttributeAwarenessMaxEntOnly() ;

  virtual void
  testKeywordAttributeAwarenessDictionaryAndMaxEnt() ;

protected:
  std::shared_ptr<TestOpenNLPLemmatizerFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestOpenNLPLemmatizerFilterFactory>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::opennlp
