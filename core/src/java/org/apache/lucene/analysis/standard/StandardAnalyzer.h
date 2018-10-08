#pragma once
#include "../StopwordAnalyzerBase.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class CharArraySet;
}

namespace org::apache::lucene::analysis::standard
{
class StandardTokenizer;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::analysis::standard
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * Filters {@link StandardTokenizer} with {@link StandardFilter}, {@link
 * LowerCaseFilter} and {@link StopFilter}, using a deque of
 * English stop words.
 */
class StandardAnalyzer final : public StopwordAnalyzerBase
{
  GET_CLASS_NAME(StandardAnalyzer)

  /** An unmodifiable set containing some common English words that are not
  usually useful for searching.*/
public:
  static const std::shared_ptr<CharArraySet> ENGLISH_STOP_WORDS_SET;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static StandardAnalyzer::StaticConstructor staticConstructor;

  /** Default maximum allowed token length */
public:
  static constexpr int DEFAULT_MAX_TOKEN_LENGTH = 255;

private:
  int maxTokenLength = DEFAULT_MAX_TOKEN_LENGTH;

  /** An unmodifiable set containing some common English words that are usually
  not useful for searching. */
public:
  static const std::shared_ptr<CharArraySet> STOP_WORDS_SET;

  /** Builds an analyzer with the given stop words.
   * @param stopWords stop words */
  StandardAnalyzer(std::shared_ptr<CharArraySet> stopWords);

  /** Builds an analyzer with the default stop words ({@link #STOP_WORDS_SET}).
   */
  StandardAnalyzer();

  /** Builds an analyzer with the stop words from the given reader.
   * @see WordlistLoader#getWordSet(Reader)
   * @param stopwords Reader to read stop words from */
  StandardAnalyzer(std::shared_ptr<Reader> stopwords) ;

  /**
   * Set the max allowed token length.  Tokens larger than this will be chopped
   * up at this token length and emitted as multiple tokens.  If you need to
   * skip such large tokens, you could increase this max length, and then
   * use {@code LengthFilter} to remove long tokens.  The default is
   * {@link StandardAnalyzer#DEFAULT_MAX_TOKEN_LENGTH}.
   */
  void setMaxTokenLength(int length);

  /** Returns the current maximum token length
   *
   *  @see #setMaxTokenLength */
  int getMaxTokenLength();

protected:
  std::shared_ptr<TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

private:
  class TokenStreamComponentsAnonymousInnerClass : public TokenStreamComponents
  {
    GET_CLASS_NAME(TokenStreamComponentsAnonymousInnerClass)
  private:
    std::shared_ptr<StandardAnalyzer> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::standard::StandardTokenizer>
        src;

  public:
    TokenStreamComponentsAnonymousInnerClass(
        std::shared_ptr<StandardAnalyzer> outerInstance,
        std::shared_ptr<TokenStream> tok,
        std::shared_ptr<
            org::apache::lucene::analysis::standard::StandardTokenizer>
            src);

  protected:
    void setReader(std::shared_ptr<Reader> reader) override;

  protected:
    std::shared_ptr<TokenStreamComponentsAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TokenStreamComponentsAnonymousInnerClass>(
          TokenStreamComponents::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<StandardAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<StandardAnalyzer>(
        org.apache.lucene.analysis.StopwordAnalyzerBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::standard
