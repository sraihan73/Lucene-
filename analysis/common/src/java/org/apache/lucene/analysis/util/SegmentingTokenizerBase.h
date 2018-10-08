#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::util
{
class CharArrayIterator;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
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
namespace org::apache::lucene::analysis::util
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Breaks text into sentences with a {@link BreakIterator} and
 * allows subclasses to decompose these sentences into words.
 * <p>
 * This can be used by subclasses that need sentence context
 * for tokenization purposes, such as CJK segmenters.
 * <p>
 * Additionally it can be used by subclasses that want to mark
 * sentence boundaries (with a custom attribute, extra token, position
 * increment, etc) for downstream processing.
 *
 * @lucene.experimental
 */
class SegmentingTokenizerBase : public Tokenizer
{
  GET_CLASS_NAME(SegmentingTokenizerBase)
protected:
  static constexpr int BUFFERMAX = 1024;
  std::deque<wchar_t> const buffer = std::deque<wchar_t>(BUFFERMAX);
  /** true length of text in the buffer */
private:
  int length = 0;
  /** length in buffer that can be evaluated safely, up to a safe end point */
  int usableLength = 0;
  /** accumulated offset of previous buffers for this reader, for offsetAtt */
protected:
  int offset = 0;

private:
  const std::shared_ptr<BreakIterator> iterator;
  const std::shared_ptr<CharArrayIterator> wrapper =
      CharArrayIterator::newSentenceInstance();

  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

  /**
   * Construct a new SegmenterBase, using
   * the provided BreakIterator for sentence segmentation.
   * <p>
   * Note that you should never share BreakIterators across different
   * TokenStreams, instead a newly created or cloned one should always
   * be provided to this constructor.
   */
public:
  SegmentingTokenizerBase(std::shared_ptr<BreakIterator> iterator);

  /**
   * Construct a new SegmenterBase, also supplying the AttributeFactory
   */
  SegmentingTokenizerBase(std::shared_ptr<AttributeFactory> factory,
                          std::shared_ptr<BreakIterator> iterator);

  bool incrementToken()  override final;

  void reset()  override;

  void end()  override final;

  /** Returns the last unambiguous break position in the text. */
private:
  int findSafeEnd();

  /** For sentence tokenization, these are the unambiguous break positions. */
protected:
  virtual bool isSafeEnd(wchar_t ch);

  /**
   * Refill the buffer, accumulating the offset and setting usableLength to the
   * last unambiguous break position
   */
private:
  void refill() ;

  // TODO: refactor to a shared readFully somewhere
  // (NGramTokenizer does this too):
  /** commons-io's readFully, but without bugs if offset != 0 */
  static int read(std::shared_ptr<Reader> input, std::deque<wchar_t> &buffer,
                  int offset, int length) ;

  /**
   * return true if there is a token from the buffer, or null if it is
   * exhausted.
   */
  bool incrementSentence() ;

  /** Provides the next input sentence for analysis */
protected:
  virtual void setNextSentence(int sentenceStart, int sentenceEnd) = 0;

  /** Returns true if another word is available */
  virtual bool incrementWord() = 0;

protected:
  std::shared_ptr<SegmentingTokenizerBase> shared_from_this()
  {
    return std::static_pointer_cast<SegmentingTokenizerBase>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::util
