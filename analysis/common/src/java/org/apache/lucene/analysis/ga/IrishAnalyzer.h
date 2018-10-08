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
namespace org::apache::lucene::analysis::ga
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * {@link Analyzer} for Irish.
 */
class IrishAnalyzer final : public StopwordAnalyzerBase
{
  GET_CLASS_NAME(IrishAnalyzer)
private:
  const std::shared_ptr<CharArraySet> stemExclusionSet;

  /** File containing default Irish stopwords. */
public:
  static const std::wstring DEFAULT_STOPWORD_FILE;

private:
  static const std::shared_ptr<CharArraySet> DEFAULT_ARTICLES;

  /**
   * When StandardTokenizer splits t‑athair into {t, athair}, we don't
   * want to cause a position increment, otherwise there will be problems
   * with phrase queries versus tAthair (which would not have a gap).
   */
  static const std::shared_ptr<CharArraySet> HYPHENATIONS;

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

  /**
   * Builds an analyzer with the default stop words: {@link
   * #DEFAULT_STOPWORD_FILE}.
   */
public:
  IrishAnalyzer();

  /**
   * Builds an analyzer with the given stop words.
   *
   * @param stopwords a stopword set
   */
  IrishAnalyzer(std::shared_ptr<CharArraySet> stopwords);

  /**
   * Builds an analyzer with the given stop words. If a non-empty stem exclusion
   * set is provided this analyzer will add a {@link SetKeywordMarkerFilter}
   * before stemming.
   *
   * @param stopwords a stopword set
   * @param stemExclusionSet a set of terms not to be stemmed
   */
  IrishAnalyzer(std::shared_ptr<CharArraySet> stopwords,
                std::shared_ptr<CharArraySet> stemExclusionSet);

  /**
   * Creates a
   * {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   * which tokenizes all the text in the provided {@link Reader}.
   *
   * @return A
   *         {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   *         built from an {@link StandardTokenizer} filtered with
   *         {@link StandardFilter}, {@link IrishLowerCaseFilter}, {@link
   * StopFilter} , {@link SetKeywordMarkerFilter} if a stem exclusion set is
   *         provided and {@link SnowballFilter}.
   */
protected:
  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<IrishAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<IrishAnalyzer>(
        org.apache.lucene.analysis.StopwordAnalyzerBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ga
