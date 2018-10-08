#pragma once
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::icu::segmentation
{
class CompositeBreakIterator;
}

namespace org::apache::lucene::analysis::icu::segmentation
{
class ICUTokenizerConfig;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
}
namespace org::apache::lucene::analysis::icu::tokenattributes
{
class ScriptAttribute;
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
namespace org::apache::lucene::analysis::icu::segmentation
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using ScriptAttribute =
    org::apache::lucene::analysis::icu::tokenattributes::ScriptAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Breaks text into words according to UAX #29: Unicode Text Segmentation
 * (http://www.unicode.org/reports/tr29/)
 * <p>
 * Words are broken across script boundaries, then segmented according to
 * the BreakIterator and typing provided by the {@link ICUTokenizerConfig}
 * </p>
 * @see ICUTokenizerConfig
 * @lucene.experimental
 */
class ICUTokenizer final : public Tokenizer
{
  GET_CLASS_NAME(ICUTokenizer)
private:
  static constexpr int IOBUFFER = 4096;
  std::deque<wchar_t> const buffer = std::deque<wchar_t>(IOBUFFER);
  /** true length of text in the buffer */
  int length = 0;
  /** length in buffer that can be evaluated safely, up to a safe end point */
  int usableLength = 0;
  /** accumulated offset of previous buffers for this reader, for offsetAtt */
  int offset = 0;

  const std::shared_ptr<CompositeBreakIterator>
      breaker; // tokenizes a char[] of text
  const std::shared_ptr<ICUTokenizerConfig> config;
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);
  const std::shared_ptr<ScriptAttribute> scriptAtt =
      addAttribute(ScriptAttribute::typeid);

  /**
   * Construct a new ICUTokenizer that breaks text into words from the given
   * Reader.
   * <p>
   * The default script-specific handling is used.
   * <p>
   * The default attribute factory is used.
   *
   * @see DefaultICUTokenizerConfig
   */
public:
  ICUTokenizer();

  /**
   * Construct a new ICUTokenizer that breaks text into words from the given
   * Reader, using a tailored BreakIterator configuration.
   * <p>
   * The default attribute factory is used.
   *
   * @param config Tailored BreakIterator configuration
   */
  ICUTokenizer(std::shared_ptr<ICUTokenizerConfig> config);

  /**
   * Construct a new ICUTokenizer that breaks text into words from the given
   * Reader, using a tailored BreakIterator configuration.
   *
   * @param factory AttributeFactory to use
   * @param config Tailored BreakIterator configuration
   */
  ICUTokenizer(std::shared_ptr<AttributeFactory> factory,
               std::shared_ptr<ICUTokenizerConfig> config);

  bool incrementToken()  override;

  void reset()  override;

  void end()  override;

  /*
   * This tokenizes text based upon the longest matching rule, and because of
   * this, isn't friendly to a Reader.
   *
   * Text is read from the input stream in 4kB chunks. Within a 4kB chunk of
   * text, the last unambiguous break point is found (in this implementation:
   * white space character) Any remaining characters represent possible partial
   * words, so are appended to the front of the next chunk.
   *
   * There is the possibility that there are no unambiguous break points within
   * an entire 4kB chunk of text (binary data). So there is a maximum word limit
   * of 4kB since it will not try to grow the buffer in this case.
   */

  /**
   * Returns the last unambiguous break position in the text.
   *
   * @return position of character, or -1 if one does not exist
   */
private:
  int findSafeEnd();

  /**
   * Refill the buffer, accumulating the offset and setting usableLength to the
   * last unambiguous break position
   *
   * @throws IOException If there is a low-level I/O error.
   */
  void refill() ;

  // TODO: refactor to a shared readFully somewhere
  // (NGramTokenizer does this too):
  /** commons-io's readFully, but without bugs if offset != 0 */
  static int read(std::shared_ptr<Reader> input, std::deque<wchar_t> &buffer,
                  int offset, int length) ;

  /*
   * return true if there is a token from the buffer, or null if it is
   * exhausted.
   */
  bool incrementTokenBuffer();

protected:
  std::shared_ptr<ICUTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<ICUTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::icu::segmentation
