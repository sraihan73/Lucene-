#pragma once
#include "stringhelper.h"
#include <memory>
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

namespace org::apache::lucene::analysis::opennlp::tools
{

using opennlp::tools::namefind::TokenNameFinder;
using opennlp::tools::namefind::TokenNameFinderModel;
using opennlp::tools::util::Span;

/**
 * Supply OpenNLP Named Entity Resolution tool
 * Requires binary models from OpenNLP project on SourceForge.
 *
 * Usage: from <a
 * href="http://opennlp.apache.org/docs/1.8.3/manual/opennlp.html#tools.namefind.recognition.api"
 *             >the OpenNLP documentation</a>:
 *
 * "The NameFinderME class is not thread safe, it must only be called from one
 * thread. To use multiple threads multiple NameFinderME instances sharing the
 * same model instance can be created. The input text should be segmented into
 * documents, sentences and tokens. To perform entity detection an application
 * calls the find method for every sentence in the document. After every
 * document clearAdaptiveData must be called to clear the adaptive data in the
 * feature generators. Not calling clearAdaptiveData can lead to a sharp drop in
 * the detection rate after a few documents."
 *
 */
class NLPNERTaggerOp : public std::enable_shared_from_this<NLPNERTaggerOp>
{
  GET_CLASS_NAME(NLPNERTaggerOp)
private:
  const std::shared_ptr<TokenNameFinder> nameFinder;

public:
  NLPNERTaggerOp(std::shared_ptr<TokenNameFinderModel> model);

  virtual std::deque<std::shared_ptr<Span>>
  getNames(std::deque<std::wstring> &words);

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void reset();
};

} // #include  "core/src/java/org/apache/lucene/analysis/opennlp/tools/
