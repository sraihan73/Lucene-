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
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
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
namespace org::apache::lucene::analysis::en
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;

/**
 * {@link Analyzer} for English.
 */
class EnglishAnalyzer final : public StopwordAnalyzerBase
{
  GET_CLASS_NAME(EnglishAnalyzer)
private:
  const std::shared_ptr<CharArraySet> stemExclusionSet;

  /**
   * Returns an unmodifiable instance of the default stop words set.
   * @return default stop words set.
   */
public:
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
  };

  /**
   * Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
   */
public:
  EnglishAnalyzer();

  /**
   * Builds an analyzer with the given stop words.
   *
   * @param stopwords a stopword set
   */
  EnglishAnalyzer(std::shared_ptr<CharArraySet> stopwords);

  /**
   * Builds an analyzer with the given stop words. If a non-empty stem exclusion
   * set is provided this analyzer will add a {@link SetKeywordMarkerFilter}
   * before stemming.
   *
   * @param stopwords a stopword set
   * @param stemExclusionSet a set of terms not to be stemmed
   */
  EnglishAnalyzer(std::shared_ptr<CharArraySet> stopwords,
                  std::shared_ptr<CharArraySet> stemExclusionSet);

  /**
   * Creates a
   * {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   * which tokenizes all the text in the provided {@link Reader}.
   *
   * @return A
   *         {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   *         built from an {@link StandardTokenizer} filtered with
   *         {@link StandardFilter}, {@link EnglishPossessiveFilter},
   *         {@link LowerCaseFilter}, {@link StopFilter}
   *         , {@link SetKeywordMarkerFilter} if a stem exclusion set is
   *         provided and {@link PorterStemFilter}.
   */
protected:
  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<EnglishAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<EnglishAnalyzer>(
        org.apache.lucene.analysis.StopwordAnalyzerBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::en
