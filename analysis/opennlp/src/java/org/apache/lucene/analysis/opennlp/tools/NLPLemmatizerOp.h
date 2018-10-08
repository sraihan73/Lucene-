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

using opennlp::tools::lemmatizer::DictionaryLemmatizer;
using opennlp::tools::lemmatizer::LemmatizerME;
using opennlp::tools::lemmatizer::LemmatizerModel;

/**
 * <p>Supply OpenNLP Lemmatizer tools.</p>
 * <p>
 *   Both a dictionary-based lemmatizer and a MaxEnt lemmatizer are supported.
 *   If both are configured, the dictionary-based lemmatizer is tried first,
 *   and then the MaxEnt lemmatizer is consulted for out-of-vocabulary tokens.
 * </p>
 * <p>
 *   The MaxEnt implementation requires binary models from OpenNLP project on
 * SourceForge.
 * </p>
 */
class NLPLemmatizerOp : public std::enable_shared_from_this<NLPLemmatizerOp>
{
  GET_CLASS_NAME(NLPLemmatizerOp)
private:
  const std::shared_ptr<DictionaryLemmatizer> dictionaryLemmatizer;
  const std::shared_ptr<LemmatizerME> lemmatizerME;

public:
  NLPLemmatizerOp(
      std::shared_ptr<InputStream> dictionary,
      std::shared_ptr<LemmatizerModel> lemmatizerModel) ;

  virtual std::deque<std::wstring>
  lemmatize(std::deque<std::wstring> &words,
            std::deque<std::wstring> &postags);
};

} // #include  "core/src/java/org/apache/lucene/analysis/opennlp/tools/
