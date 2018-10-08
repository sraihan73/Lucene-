#pragma once
#include "../../../../../../../../../common/src/java/org/apache/lucene/analysis/util/SegmentingTokenizerBase.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
}
namespace org::apache::lucene::analysis::cn::smart
{
class WordSegmenter;
}
namespace org::apache::lucene::analysis::cn::smart::hhmm
{
class SegToken;
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
namespace org::apache::lucene::analysis::cn::smart
{

using SegToken = org::apache::lucene::analysis::cn::smart::hhmm::SegToken;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using SegmentingTokenizerBase =
    org::apache::lucene::analysis::util::SegmentingTokenizerBase;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Tokenizer for Chinese or mixed Chinese-English text.
 * <p>
 * The analyzer uses probabilistic knowledge to find the optimal word
 * segmentation for Simplified Chinese text. The text is first broken into
 * sentences, then each sentence is segmented into words.
 */
class HMMChineseTokenizer : public SegmentingTokenizerBase
{
  GET_CLASS_NAME(HMMChineseTokenizer)
  /** used for breaking the text into sentences */
private:
  static const std::shared_ptr<BreakIterator> sentenceProto;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);

  const std::shared_ptr<WordSegmenter> wordSegmenter =
      std::make_shared<WordSegmenter>();
  std::shared_ptr<Iterator<std::shared_ptr<SegToken>>> tokens;

  /** Creates a new HMMChineseTokenizer */
public:
  HMMChineseTokenizer();

  /** Creates a new HMMChineseTokenizer, supplying the AttributeFactory */
  HMMChineseTokenizer(std::shared_ptr<AttributeFactory> factory);

protected:
  void setNextSentence(int sentenceStart, int sentenceEnd) override;

  bool incrementWord() override;

public:
  void reset()  override;

protected:
  std::shared_ptr<HMMChineseTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<HMMChineseTokenizer>(
        org.apache.lucene.analysis.util
            .SegmentingTokenizerBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::cn::smart
