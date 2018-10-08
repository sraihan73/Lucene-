#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/icu/ReplaceableTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::analysis::icu
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

using com::ibm::icu::text::Replaceable;
using com::ibm::icu::text::Transliterator;

/**
 * A {@link TokenFilter} that transforms text with ICU.
 * <p>
 * ICU provides text-transformation functionality via its Transliteration API.
 * Although script conversion is its most common use, a Transliterator can
 * actually perform a more general class of tasks. In fact, Transliterator
 * defines a very general API which specifies only that a segment of the input
 * text is replaced by new text. The particulars of this conversion are
 * determined entirely by subclasses of Transliterator.
 * </p>
 * <p>
 * Some useful transformations for search are built-in:
 * <ul>
 * <li>Conversion from Traditional to Simplified Chinese characters
 * <li>Conversion from Hiragana to Katakana
 * <li>Conversion from Fullwidth to Halfwidth forms.
 * <li>Script conversions, for example Serbian Cyrillic to Latin
 * </ul>
 * <p>
 * Example usage: <blockquote>stream = new ICUTransformFilter(stream,
 * Transliterator.getInstance("Traditional-Simplified"));</blockquote>
 * <br>
 * For more details, see the <a
 * href="http://userguide.icu-project.org/transforms/general">ICU User
 * Guide</a>.
 */
class ICUTransformFilter final : public TokenFilter
{
  GET_CLASS_NAME(ICUTransformFilter)
  // Transliterator to transform the text
private:
  const std::shared_ptr<Transliterator> transform;

  // Reusable position object
  const std::shared_ptr<Transliterator::Position> position =
      std::make_shared<Transliterator::Position>();

  // term attribute, will be updated with transformed text.
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

  // Wraps a termAttribute around the replaceable interface.
  const std::shared_ptr<ReplaceableTermAttribute> replaceableAttribute =
      std::make_shared<ReplaceableTermAttribute>();

  /**
   * Create a new ICUTransformFilter that transforms text on the given stream.
   *
   * @param input {@link TokenStream} to filter.
   * @param transform Transliterator to transform the text.
   */
public:
  ICUTransformFilter(std::shared_ptr<TokenStream> input,
                     std::shared_ptr<Transliterator> transform);

  bool incrementToken()  override;

  /**
   * Wrap a {@link CharTermAttribute} with the Replaceable API.
   */
public:
  class ReplaceableTermAttribute final
      : public std::enable_shared_from_this<ReplaceableTermAttribute>,
        public Replaceable
  {
    GET_CLASS_NAME(ReplaceableTermAttribute)
  private:
    std::deque<wchar_t> buffer;
    // C++ NOTE: Fields cannot have the same name as methods:
    int length_ = 0;
    std::shared_ptr<CharTermAttribute> token;

  public:
    void setText(std::shared_ptr<CharTermAttribute> token);

    int char32At(int pos) override;

    wchar_t charAt(int pos) override;

    void copy(int start, int limit, int dest) override;

    void getChars(int srcStart, int srcLimit, std::deque<wchar_t> &dst,
                  int dstStart) override;

    bool hasMetaData() override;

    int length() override;

    void replace(int start, int limit, const std::wstring &text) override;

    void replace(int start, int limit, std::deque<wchar_t> &text,
                 int charsStart, int charsLen) override;

    /** shift text (if necessary) for a replacement operation */
  private:
    int shiftForReplace(int start, int limit, int charsLen);
  };

protected:
  std::shared_ptr<ICUTransformFilter> shared_from_this()
  {
    return std::static_pointer_cast<ICUTransformFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/icu/
