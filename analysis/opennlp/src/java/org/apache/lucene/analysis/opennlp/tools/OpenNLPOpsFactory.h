#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/opennlp/tools/NLPSentenceDetectorOp.h"

#include  "core/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include  "core/src/java/org/apache/lucene/analysis/opennlp/tools/NLPTokenizerOp.h"
#include  "core/src/java/org/apache/lucene/analysis/opennlp/tools/NLPPOSTaggerOp.h"
#include  "core/src/java/org/apache/lucene/analysis/opennlp/tools/NLPChunkerOp.h"
#include  "core/src/java/org/apache/lucene/analysis/opennlp/tools/NLPNERTaggerOp.h"
#include  "core/src/java/org/apache/lucene/analysis/opennlp/tools/NLPLemmatizerOp.h"

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

namespace org::apache::lucene::analysis::opennlp::tools
{

using opennlp::tools::chunker::ChunkerModel;
using opennlp::tools::lemmatizer::LemmatizerModel;
using opennlp::tools::namefind::TokenNameFinderModel;
using opennlp::tools::postag::POSModel;
using opennlp::tools::sentdetect::SentenceModel;
using opennlp::tools::tokenize::TokenizerModel;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;

/**
 * Supply OpenNLP Named Entity Recognizer
 * Cache model file objects. Assumes model files are thread-safe.
 */
class OpenNLPOpsFactory : public std::enable_shared_from_this<OpenNLPOpsFactory>
{
  GET_CLASS_NAME(OpenNLPOpsFactory)
private:
  static std::unordered_map<std::wstring, std::shared_ptr<SentenceModel>>
      sentenceModels;
  static std::shared_ptr<
      ConcurrentHashMap<std::wstring, std::shared_ptr<TokenizerModel>>>
      tokenizerModels;
  static std::shared_ptr<
      ConcurrentHashMap<std::wstring, std::shared_ptr<POSModel>>>
      posTaggerModels;
  static std::shared_ptr<
      ConcurrentHashMap<std::wstring, std::shared_ptr<ChunkerModel>>>
      chunkerModels;
  static std::unordered_map<std::wstring, std::shared_ptr<TokenNameFinderModel>>
      nerModels;
  static std::unordered_map<std::wstring, std::shared_ptr<LemmatizerModel>>
      lemmatizerModels;
  static std::unordered_map<std::wstring, std::wstring> lemmaDictionaries;

public:
  static std::shared_ptr<NLPSentenceDetectorOp>
  getSentenceDetector(const std::wstring &modelName) ;

  static std::shared_ptr<SentenceModel>
  getSentenceModel(const std::wstring &modelName,
                   std::shared_ptr<ResourceLoader> loader) ;

  static std::shared_ptr<NLPTokenizerOp>
  getTokenizer(const std::wstring &modelName) ;

  static std::shared_ptr<TokenizerModel>
  getTokenizerModel(const std::wstring &modelName,
                    std::shared_ptr<ResourceLoader> loader) ;

  static std::shared_ptr<NLPPOSTaggerOp>
  getPOSTagger(const std::wstring &modelName) ;

  static std::shared_ptr<POSModel>
  getPOSTaggerModel(const std::wstring &modelName,
                    std::shared_ptr<ResourceLoader> loader) ;

  static std::shared_ptr<NLPChunkerOp>
  getChunker(const std::wstring &modelName) ;

  static std::shared_ptr<ChunkerModel>
  getChunkerModel(const std::wstring &modelName,
                  std::shared_ptr<ResourceLoader> loader) ;

  static std::shared_ptr<NLPNERTaggerOp>
  getNERTagger(const std::wstring &modelName) ;

  static std::shared_ptr<TokenNameFinderModel>
  getNERTaggerModel(const std::wstring &modelName,
                    std::shared_ptr<ResourceLoader> loader) ;

  static std::shared_ptr<NLPLemmatizerOp>
  getLemmatizer(const std::wstring &dictionaryFile,
                const std::wstring &lemmatizerModelFile) ;

  static std::wstring getLemmatizerDictionary(
      const std::wstring &dictionaryFile,
      std::shared_ptr<ResourceLoader> loader) ;

  static std::shared_ptr<LemmatizerModel>
  getLemmatizerModel(const std::wstring &modelName,
                     std::shared_ptr<ResourceLoader> loader) ;

  // keeps unit test from blowing out memory
  static void clearModels();
};

} // #include  "core/src/java/org/apache/lucene/analysis/opennlp/tools/
