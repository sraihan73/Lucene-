#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopwordAnalyzerBase.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

#include  "core/src/java/org/egothor/stemmer/Trie.h"
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
namespace org::apache::lucene::analysis::pl
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Trie = org::egothor::stemmer::Trie;

/**
 * {@link Analyzer} for Polish.
 */
class PolishAnalyzer final : public StopwordAnalyzerBase
{
  GET_CLASS_NAME(PolishAnalyzer)
private:
  const std::shared_ptr<CharArraySet> stemExclusionSet;
  const std::shared_ptr<Trie> stemTable;

  /** File containing default Polish stopwords. */
public:
  static const std::wstring DEFAULT_STOPWORD_FILE;

  /** File containing default Polish stemmer table. */
  static const std::wstring DEFAULT_STEMMER_FILE;

  /**
   * Returns an unmodifiable instance of the default stop words set.
   * @return default stop words set.
   */
  static std::shared_ptr<CharArraySet> getDefaultStopSet();

  /**
   * Returns an unmodifiable instance of the default stemmer table.
   */
  static std::shared_ptr<Trie> getDefaultTable();

  /**
   * Atomically loads the DEFAULT_STOP_SET in a lazy fashion once the outer
   * class accesses the static final set the first time.;
   */
private:
  class DefaultsHolder : public std::enable_shared_from_this<DefaultsHolder>
  {
    GET_CLASS_NAME(DefaultsHolder)
  public:
    static const std::shared_ptr<CharArraySet> DEFAULT_STOP_SET;
    static const std::shared_ptr<Trie> DEFAULT_TABLE;

  private:
    class StaticConstructor
        : public std::enable_shared_from_this<StaticConstructor>
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

  private:
    static DefaultsHolder::StaticConstructor staticConstructor;
  };

  /**
   * Builds an analyzer with the default stop words: {@link
   * #DEFAULT_STOPWORD_FILE}.
   */
public:
  PolishAnalyzer();

  /**
   * Builds an analyzer with the given stop words.
   *
   * @param stopwords a stopword set
   */
  PolishAnalyzer(std::shared_ptr<CharArraySet> stopwords);

  /**
   * Builds an analyzer with the given stop words. If a non-empty stem exclusion
   * set is provided this analyzer will add a {@link SetKeywordMarkerFilter}
   * before stemming.
   *
   * @param stopwords a stopword set
   * @param stemExclusionSet a set of terms not to be stemmed
   */
  PolishAnalyzer(std::shared_ptr<CharArraySet> stopwords,
                 std::shared_ptr<CharArraySet> stemExclusionSet);

  /**
   * Creates a
   * {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   * which tokenizes all the text in the provided {@link Reader}.
   *
   * @return A
   *         {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   *         built from an {@link StandardTokenizer} filtered with
   *         {@link StandardFilter}, {@link LowerCaseFilter}, {@link StopFilter}
   *         , {@link SetKeywordMarkerFilter} if a stem exclusion set is
   *         provided and {@link StempelFilter}.
   */
protected:
  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<PolishAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<PolishAnalyzer>(
        org.apache.lucene.analysis.StopwordAnalyzerBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/pl/
