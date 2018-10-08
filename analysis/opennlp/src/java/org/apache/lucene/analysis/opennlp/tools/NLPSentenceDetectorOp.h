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

using opennlp::tools::sentdetect::SentenceDetectorME;
using opennlp::tools::sentdetect::SentenceModel;
using opennlp::tools::util::Span;

/**
 * Supply OpenNLP Sentence Detector tool
 * Requires binary models from OpenNLP project on SourceForge.
 */
class NLPSentenceDetectorOp
    : public std::enable_shared_from_this<NLPSentenceDetectorOp>
{
  GET_CLASS_NAME(NLPSentenceDetectorOp)
private:
  const std::shared_ptr<SentenceDetectorME> sentenceSplitter;

public:
  NLPSentenceDetectorOp(std::shared_ptr<SentenceModel> model) throw(
      IOException);

  NLPSentenceDetectorOp();

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::deque<std::shared_ptr<Span>>
  splitSentences(const std::wstring &line);
};

} // #include  "core/src/java/org/apache/lucene/analysis/opennlp/tools/
