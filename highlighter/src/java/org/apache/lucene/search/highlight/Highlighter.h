#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::highlight
{
class Formatter;
}

namespace org::apache::lucene::search::highlight
{
class Encoder;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search::highlight
{
class Scorer;
}
namespace org::apache::lucene::search::highlight
{
class Fragmenter;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::search::highlight
{
class InvalidTokenOffsetsException;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::search::highlight
{
class TextFragment;
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
namespace org::apache::lucene::search::highlight
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Marks up highlighted terms found in the best sections of
 * text, using configurable {@link Fragmenter}, {@link Scorer}, {@link
 * Formatter},
 * {@link Encoder} and tokenizers.
 *
 * This is Lucene's original Highlighter; there are others.
 */
class Highlighter : public std::enable_shared_from_this<Highlighter>
{
  GET_CLASS_NAME(Highlighter)
public:
  static constexpr int DEFAULT_MAX_CHARS_TO_ANALYZE = 50 * 1024;

private:
  std::shared_ptr<Formatter> formatter;
  std::shared_ptr<Encoder> encoder;
  std::shared_ptr<Scorer> fragmentScorer;
  int maxDocCharsToAnalyze = DEFAULT_MAX_CHARS_TO_ANALYZE;
  std::shared_ptr<Fragmenter> textFragmenter =
      std::make_shared<SimpleFragmenter>();

public:
  Highlighter(std::shared_ptr<Scorer> fragmentScorer);

  Highlighter(std::shared_ptr<Formatter> formatter,
              std::shared_ptr<Scorer> fragmentScorer);

  Highlighter(std::shared_ptr<Formatter> formatter,
              std::shared_ptr<Encoder> encoder,
              std::shared_ptr<Scorer> fragmentScorer);

  /**
   * Highlights chosen terms in a text, extracting the most relevant section.
   * This is a convenience method that calls
   * {@link #getBestFragment(TokenStream, std::wstring)}
   *
   * @param analyzer   the analyzer that will be used to split <code>text</code>
   * into chunks
   * @param text text to highlight terms in
   * @param fieldName Name of field used to influence analyzer's tokenization
   * policy
   *
   * @return highlighted text fragment or null if no terms found
   * @throws InvalidTokenOffsetsException thrown if any token's endOffset
   * exceeds the provided text's length
   */
  std::wstring
  getBestFragment(std::shared_ptr<Analyzer> analyzer,
                  const std::wstring &fieldName,
                  const std::wstring &text) throw(IOException,
                                                  InvalidTokenOffsetsException);

  /**
   * Highlights chosen terms in a text, extracting the most relevant section.
   * The document text is analysed in chunks to record hit statistics
   * across the document. After accumulating stats, the fragment with the
   * highest score is returned
   *
   * @param tokenStream   a stream of tokens identified in the text parameter,
   * including offset information. This is typically produced by an analyzer
   * re-parsing a document's text. Some work may be done on retrieving
   * TokenStreams more efficiently by adding support for storing original text
   * position data in the Lucene index but this support is not currently
   * available (as of Lucene 1.4 rc2).
   * @param text text to highlight terms in
   *
   * @return highlighted text fragment or null if no terms found
   * @throws InvalidTokenOffsetsException thrown if any token's endOffset
   * exceeds the provided text's length
   */
  std::wstring
  getBestFragment(std::shared_ptr<TokenStream> tokenStream,
                  const std::wstring &text) throw(IOException,
                                                  InvalidTokenOffsetsException);

  /**
   * Highlights chosen terms in a text, extracting the most relevant sections.
   * This is a convenience method that calls
   * {@link #getBestFragments(TokenStream, std::wstring, int)}
   *
   * @param analyzer   the analyzer that will be used to split <code>text</code>
   * into chunks
   * @param fieldName     the name of the field being highlighted (used by
   * analyzer)
   * @param text          text to highlight terms in
   * @param maxNumFragments  the maximum number of fragments.
   *
   * @return highlighted text fragments (between 0 and maxNumFragments number of
   * fragments)
   * @throws InvalidTokenOffsetsException thrown if any token's endOffset
   * exceeds the provided text's length
   */
  std::deque<std::wstring>
  getBestFragments(std::shared_ptr<Analyzer> analyzer,
                   const std::wstring &fieldName, const std::wstring &text,
                   int maxNumFragments) throw(IOException,
                                              InvalidTokenOffsetsException);

  /**
   * Highlights chosen terms in a text, extracting the most relevant sections.
   * The document text is analysed in chunks to record hit statistics
   * across the document. After accumulating stats, the fragments with the
   * highest scores are returned as an array of strings in order of score
   * (contiguous fragments are merged into one in their original order to
   * improve readability)
   *
   * @param text          text to highlight terms in
   * @param maxNumFragments  the maximum number of fragments.
   *
   * @return highlighted text fragments (between 0 and maxNumFragments number of
   * fragments)
   * @throws InvalidTokenOffsetsException thrown if any token's endOffset
   * exceeds the provided text's length
   */
  std::deque<std::wstring> getBestFragments(
      std::shared_ptr<TokenStream> tokenStream, const std::wstring &text,
      int maxNumFragments) ;

  /**
   * Low level api to get the most relevant (formatted) sections of the
   * document. This method has been made public to allow visibility of score
   * information held in TextFragment objects. Thanks to Jason Calabrese for
   * help in redefining the interface.
   * @throws IOException If there is a low-level I/O error
   * @throws InvalidTokenOffsetsException thrown if any token's endOffset
   * exceeds the provided text's length
   */
  std::deque<std::shared_ptr<TextFragment>>
  getBestTextFragments(std::shared_ptr<TokenStream> tokenStream,
                       const std::wstring &text, bool mergeContiguousFragments,
                       int maxNumFragments) throw(IOException,
                                                  InvalidTokenOffsetsException);

  /** Improves readability of a score-sorted deque of TextFragments by merging
   * any fragments that were contiguous in the original text into one larger
   * fragment with the correct order. This will leave a "null" in the array
   * entry for the lesser scored fragment.
   *
   * @param frag An array of document fragments in descending score
   */
private:
  void
  mergeContiguousFragments(std::deque<std::shared_ptr<TextFragment>> &frag);

  /**
   * Highlights terms in the  text , extracting the most relevant sections
   * and concatenating the chosen fragments with a separator (typically "...").
   * The document text is analysed in chunks to record hit statistics
   * across the document. After accumulating stats, the fragments with the
   * highest scores are returned in order as "separator" delimited strings.
   *
   * @param text        text to highlight terms in
   * @param maxNumFragments  the maximum number of fragments.
   * @param separator  the separator used to intersperse the document fragments
   * (typically "...")
   *
   * @return highlighted text
   * @throws InvalidTokenOffsetsException thrown if any token's endOffset
   * exceeds the provided text's length
   */
public:
  std::wstring getBestFragments(
      std::shared_ptr<TokenStream> tokenStream, const std::wstring &text,
      int maxNumFragments,
      const std::wstring &separator) throw(IOException,
                                           InvalidTokenOffsetsException);

  virtual int getMaxDocCharsToAnalyze();

  virtual void setMaxDocCharsToAnalyze(int maxDocCharsToAnalyze);

  virtual std::shared_ptr<Fragmenter> getTextFragmenter();

  virtual void setTextFragmenter(std::shared_ptr<Fragmenter> fragmenter);

  /**
   * @return Object used to score each text fragment
   */
  virtual std::shared_ptr<Scorer> getFragmentScorer();

  virtual void setFragmentScorer(std::shared_ptr<Scorer> scorer);

  virtual std::shared_ptr<Encoder> getEncoder();

  virtual void setEncoder(std::shared_ptr<Encoder> encoder);

  /**
   * Throws an IllegalArgumentException with the provided message if 'argument'
   * is null.
   *
   * @param argument the argument to be null-checked
   * @param message  the message of the exception thrown if argument == null
   */
private:
  static void ensureArgumentNotNull(std::any argument,
                                    const std::wstring &message);

public:
  class FragmentQueue : public PriorityQueue<std::shared_ptr<TextFragment>>
  {
    GET_CLASS_NAME(FragmentQueue)
  public:
    FragmentQueue(int size);

    bool lessThan(std::shared_ptr<TextFragment> fragA,
                  std::shared_ptr<TextFragment> fragB) override;

  protected:
    std::shared_ptr<FragmentQueue> shared_from_this()
    {
      return std::static_pointer_cast<FragmentQueue>(
          org.apache.lucene.util
              .PriorityQueue<TextFragment>::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::search::highlight
