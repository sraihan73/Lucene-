#pragma once
#include "FilteringTokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
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
namespace org::apache::lucene::analysis
{

using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

/**
 * Removes stop words from a token stream.
 */
class StopFilter : public FilteringTokenFilter
{
  GET_CLASS_NAME(StopFilter)

private:
  const std::shared_ptr<CharArraySet> stopWords;
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

  /**
   * Constructs a filter which removes words from the input TokenStream that are
   * named in the Set.
   *
   * @param in
   *          Input stream
   * @param stopWords
   *          A {@link CharArraySet} representing the stopwords.
   * @see #makeStopSet(java.lang.std::wstring...)
   */
public:
  StopFilter(std::shared_ptr<TokenStream> in_,
             std::shared_ptr<CharArraySet> stopWords);

  /**
   * Builds a Set from an array of stop words,
   * appropriate for passing into the StopFilter constructor.
   * This permits this stopWords construction to be cached once when
   * an Analyzer is constructed.
   *
   * @param stopWords An array of stopwords
   * @see #makeStopSet(java.lang.std::wstring[], bool) passing false to ignoreCase
   */
  static std::shared_ptr<CharArraySet>
  makeStopSet(std::deque<std::wstring> &stopWords);

  /**
   * Builds a Set from an array of stop words,
   * appropriate for passing into the StopFilter constructor.
   * This permits this stopWords construction to be cached once when
   * an Analyzer is constructed.
   *
   * @param stopWords A List of Strings or char[] or any other toString()-able
   * deque representing the stopwords
   * @return A Set ({@link CharArraySet}) containing the words
   * @see #makeStopSet(java.lang.std::wstring[], bool) passing false to ignoreCase
   */
  template <typename T1>
  static std::shared_ptr<CharArraySet> makeStopSet(std::deque<T1> stopWords);

  /**
   * Creates a stopword set from the given stopword array.
   *
   * @param stopWords An array of stopwords
   * @param ignoreCase If true, all words are lower cased first.
   * @return a Set containing the words
   */
  static std::shared_ptr<CharArraySet>
  makeStopSet(std::deque<std::wstring> &stopWords, bool ignoreCase);

  /**
   * Creates a stopword set from the given stopword deque.
   * @param stopWords A List of Strings or char[] or any other toString()-able
   * deque representing the stopwords
   * @param ignoreCase if true, all words are lower cased first
   * @return A Set ({@link CharArraySet}) containing the words
   */
  template <typename T1>
  static std::shared_ptr<CharArraySet> makeStopSet(std::deque<T1> stopWords,
                                                   bool ignoreCase);

  /**
   * Returns the next input Token whose term() is not a stop word.
   */
protected:
  bool accept() override;

protected:
  std::shared_ptr<StopFilter> shared_from_this()
  {
    return std::static_pointer_cast<StopFilter>(
        org.apache.lucene.analysis.FilteringTokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
