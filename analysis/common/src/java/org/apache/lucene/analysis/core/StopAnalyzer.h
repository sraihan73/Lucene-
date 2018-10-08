#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopwordAnalyzerBase.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class CharArraySet;
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
namespace org::apache::lucene::analysis::core
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;

/**
 * Filters {@link LetterTokenizer} with {@link LowerCaseFilter} and {@link
 * StopFilter}.
 */
class StopAnalyzer final : public StopwordAnalyzerBase
{
  GET_CLASS_NAME(StopAnalyzer)

  /** An unmodifiable set containing some common English words that are not
  usually useful for searching.*/
public:
  static const std::shared_ptr<CharArraySet> ENGLISH_STOP_WORDS_SET;

  /** Builds an analyzer which removes words in
   *  {@link #ENGLISH_STOP_WORDS_SET}.
   */
  StopAnalyzer();

  /** Builds an analyzer with the stop words from the given set.
   * @param stopWords Set of stop words */
  StopAnalyzer(std::shared_ptr<CharArraySet> stopWords);

  /** Builds an analyzer with the stop words from the given path.
   * @see WordlistLoader#getWordSet(Reader)
   * @param stopwordsFile File to load stop words from */
  StopAnalyzer(std::shared_ptr<Path> stopwordsFile) ;

  /** Builds an analyzer with the stop words from the given reader.
   * @see WordlistLoader#getWordSet(Reader)
   * @param stopwords Reader to load stop words from */
  StopAnalyzer(std::shared_ptr<Reader> stopwords) ;

  /**
   * Creates
   * {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   * used to tokenize all the text in the provided {@link Reader}.
   *
   * @return {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   *         built from a {@link LowerCaseTokenizer} filtered with
   *         {@link StopFilter}
   */
protected:
  std::shared_ptr<TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<StopAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<StopAnalyzer>(
        org.apache.lucene.analysis.StopwordAnalyzerBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::core
