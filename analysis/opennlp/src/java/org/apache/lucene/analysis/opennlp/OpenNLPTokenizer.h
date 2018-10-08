#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/SegmentingTokenizerBase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class FlagsAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::opennlp::tools
{
class NLPSentenceDetectorOp;
}
namespace org::apache::lucene::analysis::opennlp::tools
{
class NLPTokenizerOp;
}
namespace org::apache::lucene::util
{
class AttributeFactory;
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

namespace org::apache::lucene::analysis::opennlp
{

using opennlp::tools::util::Span;

using NLPSentenceDetectorOp =
    org::apache::lucene::analysis::opennlp::tools::NLPSentenceDetectorOp;
using NLPTokenizerOp =
    org::apache::lucene::analysis::opennlp::tools::NLPTokenizerOp;
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using SegmentingTokenizerBase =
    org::apache::lucene::analysis::util::SegmentingTokenizerBase;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Run OpenNLP SentenceDetector and Tokenizer.
 * The last token in each sentence is marked by setting the {@link
 * #EOS_FLAG_BIT} in the FlagsAttribute; following filters can use this
 * information to apply operations to tokens one sentence at a time.
 */
class OpenNLPTokenizer final : public SegmentingTokenizerBase
{
  GET_CLASS_NAME(OpenNLPTokenizer)
public:
  static int EOS_FLAG_BIT;

private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<FlagsAttribute> flagsAtt =
      addAttribute(FlagsAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

  std::deque<std::shared_ptr<Span>> termSpans;
  int termNum = 0;
  int sentenceStart = 0;

  std::shared_ptr<NLPSentenceDetectorOp> sentenceOp = nullptr;
  std::shared_ptr<NLPTokenizerOp> tokenizerOp = nullptr;

public:
  OpenNLPTokenizer(
      std::shared_ptr<AttributeFactory> factory,
      std::shared_ptr<NLPSentenceDetectorOp> sentenceOp,
      std::shared_ptr<NLPTokenizerOp> tokenizerOp) ;

  virtual ~OpenNLPTokenizer();

protected:
  void setNextSentence(int sentenceStart, int sentenceEnd) override;

  bool incrementWord() override;

public:
  void reset()  override;

protected:
  std::shared_ptr<OpenNLPTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<OpenNLPTokenizer>(
        org.apache.lucene.analysis.util
            .SegmentingTokenizerBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::opennlp
