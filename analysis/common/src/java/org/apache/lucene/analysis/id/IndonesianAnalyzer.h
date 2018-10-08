#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopwordAnalyzerBase.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::analysis::id
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * Analyzer for Indonesian (Bahasa)
 */
class IndonesianAnalyzer final : public StopwordAnalyzerBase
{
  GET_CLASS_NAME(IndonesianAnalyzer)
  /** File containing default Indonesian stopwords. */
public:
  static const std::wstring DEFAULT_STOPWORD_FILE;

  /**
   * Returns an unmodifiable instance of the default stop-words set.
   * @return an unmodifiable instance of the default stop-words set.
   */
  static std::shared_ptr<CharArraySet> getDefaultStopSet();

  /**
   * Atomically loads the DEFAULT_STOP_SET in a lazy fashion once the outer
   * class accesses the static final set the first time.;
   */
private:
  class DefaultSetHolder : public std::enable_shared_from_this<DefaultSetHolder>
  {
    GET_CLASS_NAME(DefaultSetHolder)
  public:
    static const std::shared_ptr<CharArraySet> DEFAULT_STOP_SET;

  private:
    class StaticConstructor
        : public std::enable_shared_from_this<StaticConstructor>
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

  private:
    static DefaultSetHolder::StaticConstructor staticConstructor;
  };

private:
  const std::shared_ptr<CharArraySet> stemExclusionSet;

  /**
   * Builds an analyzer with the default stop words: {@link
   * #DEFAULT_STOPWORD_FILE}.
   */
public:
  IndonesianAnalyzer();

  /**
   * Builds an analyzer with the given stop words
   *
   * @param stopwords
   *          a stopword set
   */
  IndonesianAnalyzer(std::shared_ptr<CharArraySet> stopwords);

  /**
   * Builds an analyzer with the given stop word. If a none-empty stem exclusion
   * set is provided this analyzer will add a {@link SetKeywordMarkerFilter}
   * before
   * {@link IndonesianStemFilter}.
   *
   * @param stopwords
   *          a stopword set
   * @param stemExclusionSet
   *          a set of terms not to be stemmed
   */
  IndonesianAnalyzer(std::shared_ptr<CharArraySet> stopwords,
                     std::shared_ptr<CharArraySet> stemExclusionSet);

  /**
   * Creates
   * {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   * used to tokenize all the text in the provided {@link Reader}.
   *
   * @return {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   *         built from an {@link StandardTokenizer} filtered with
   *         {@link StandardFilter}, {@link LowerCaseFilter},
   *         {@link StopFilter}, {@link SetKeywordMarkerFilter}
   *         if a stem exclusion set is provided and {@link
   * IndonesianStemFilter}.
   */
protected:
  std::shared_ptr<TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<IndonesianAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<IndonesianAnalyzer>(
        org.apache.lucene.analysis.StopwordAnalyzerBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::id