#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopwordAnalyzerBase.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

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
namespace org::apache::lucene::analysis::fr
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * {@link Analyzer} for French language.
 * <p>
 * Supports an external deque of stopwords (words that
 * will not be indexed at all) and an external deque of exclusions (word that
will
 * not be stemmed, but indexed).
 * A default set of stopwords is used unless an alternative deque is specified,
but the
 * exclusion deque is empty by default.
 * </p>
 *
 * <p><b>NOTE</b>: This class uses the same {@link
org.apache.lucene.util.Version}
 * dependent settings as {@link StandardAnalyzer}.</p>
GET_CLASS_NAME(uses)
 */
class FrenchAnalyzer final : public StopwordAnalyzerBase
{
  GET_CLASS_NAME(FrenchAnalyzer)

  /** File containing default French stopwords. */
public:
  static const std::wstring DEFAULT_STOPWORD_FILE;

  /** Default set of articles for ElisionFilter */
  static const std::shared_ptr<CharArraySet> DEFAULT_ARTICLES;

  /**
   * Contains words that should be indexed but not stemmed.
   */
private:
  const std::shared_ptr<CharArraySet> excltable;

  /**
   * Returns an unmodifiable instance of the default stop-words set.
   * @return an unmodifiable instance of the default stop-words set.
   */
public:
  static std::shared_ptr<CharArraySet> getDefaultStopSet();

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
   * Builds an analyzer with the default stop words ({@link
   * #getDefaultStopSet}).
   */
public:
  FrenchAnalyzer();

  /**
   * Builds an analyzer with the given stop words
   *
   * @param stopwords
   *          a stopword set
   */
  FrenchAnalyzer(std::shared_ptr<CharArraySet> stopwords);

  /**
   * Builds an analyzer with the given stop words
   *
   * @param stopwords
   *          a stopword set
   * @param stemExclutionSet
   *          a stemming exclusion set
   */
  FrenchAnalyzer(std::shared_ptr<CharArraySet> stopwords,
                 std::shared_ptr<CharArraySet> stemExclutionSet);

  /**
   * Creates
   * {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   * used to tokenize all the text in the provided {@link Reader}.
   *
   * @return {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   *         built from a {@link StandardTokenizer} filtered with
   *         {@link StandardFilter}, {@link ElisionFilter},
   *         {@link LowerCaseFilter}, {@link StopFilter},
   *         {@link SetKeywordMarkerFilter} if a stem exclusion set is
   *         provided, and {@link FrenchLightStemFilter}
   */
protected:
  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<FrenchAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<FrenchAnalyzer>(
        org.apache.lucene.analysis.StopwordAnalyzerBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/fr/
