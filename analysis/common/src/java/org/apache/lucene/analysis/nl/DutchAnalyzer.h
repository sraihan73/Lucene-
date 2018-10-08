#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArrayMap.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

#include  "core/src/java/org/apache/lucene/analysis/miscellaneous/StemmerOverrideFilter.h"
#include  "core/src/java/org/apache/lucene/analysis/miscellaneous/StemmerOverrideMap.h"
namespace org::apache::lucene::analysis
{
template <typename V>
class CharArrayMap;
}
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
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
namespace org::apache::lucene::analysis::nl
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArrayMap = org::apache::lucene::analysis::CharArrayMap;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StemmerOverrideMap = org::apache::lucene::analysis::miscellaneous::
    StemmerOverrideFilter::StemmerOverrideMap;
using StemmerOverrideFilter =
    org::apache::lucene::analysis::miscellaneous::StemmerOverrideFilter;

/**
 * {@link Analyzer} for Dutch language.
 * <p>
 * Supports an external deque of stopwords (words that
 * will not be indexed at all), an external deque of exclusions (word that will
 * not be stemmed, but indexed) and an external deque of word-stem pairs that
 * overrule the algorithm (dictionary stemming). A default set of stopwords is
 * used unless an alternative deque is specified, but the exclusion deque is empty
 * by default.
 * </p>
 */
// TODO: extend StopwordAnalyzerBase
class DutchAnalyzer final : public Analyzer
{
  GET_CLASS_NAME(DutchAnalyzer)

  /** File containing default Dutch stopwords. */
public:
  static const std::wstring DEFAULT_STOPWORD_FILE;

  /**
   * Returns an unmodifiable instance of the default stop-words set.
   * @return an unmodifiable instance of the default stop-words set.
   */
  static std::shared_ptr<CharArraySet> getDefaultStopSet();

private:
  class DefaultSetHolder : public std::enable_shared_from_this<DefaultSetHolder>
  {
    GET_CLASS_NAME(DefaultSetHolder)
  public:
    static const std::shared_ptr<CharArraySet> DEFAULT_STOP_SET;
    static const std::shared_ptr<CharArrayMap<std::wstring>> DEFAULT_STEM_DICT;

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
   * Contains the stopwords used with the StopFilter.
   */
private:
  const std::shared_ptr<CharArraySet> stoptable;

  /**
   * Contains words that should be indexed but not stemmed.
   */
  std::shared_ptr<CharArraySet> excltable = CharArraySet::EMPTY_SET;

  const std::shared_ptr<StemmerOverrideMap> stemdict;

  /**
   * Builds an analyzer with the default stop words ({@link
   * #getDefaultStopSet()}) and a few default entries for the stem exclusion
   * table.
   *
   */
public:
  DutchAnalyzer();

  DutchAnalyzer(std::shared_ptr<CharArraySet> stopwords);

  DutchAnalyzer(std::shared_ptr<CharArraySet> stopwords,
                std::shared_ptr<CharArraySet> stemExclusionTable);

  DutchAnalyzer(std::shared_ptr<CharArraySet> stopwords,
                std::shared_ptr<CharArraySet> stemExclusionTable,
                std::shared_ptr<CharArrayMap<std::wstring>> stemOverrideDict);

  /**
   * Returns a (possibly reused) {@link TokenStream} which tokenizes all the
   * text in the provided {@link Reader}.
   *
   * @return A {@link TokenStream} built from a {@link StandardTokenizer}
   *   filtered with {@link StandardFilter}, {@link LowerCaseFilter},
   *   {@link StopFilter}, {@link SetKeywordMarkerFilter} if a stem exclusion
   * set is provided,
   *   {@link StemmerOverrideFilter}, and {@link SnowballFilter}
   */
protected:
  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<DutchAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<DutchAnalyzer>(
        org.apache.lucene.analysis.Analyzer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/nl/
