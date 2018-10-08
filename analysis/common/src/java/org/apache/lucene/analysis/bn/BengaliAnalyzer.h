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
namespace org::apache::lucene::analysis::bn
{

using namespace org::apache::lucene::analysis;

/**
 * Analyzer for Bengali.
 */
class BengaliAnalyzer final : public StopwordAnalyzerBase
{
  GET_CLASS_NAME(BengaliAnalyzer)
private:
  const std::shared_ptr<CharArraySet> stemExclusionSet;

  /**
   * File containing default Bengali stopwords.
   *
   * Default stopword deque is from
   * http://members.unine.ch/jacques.savoy/clef/bengaliST.txt The stopword deque
   * is BSD-Licensed.
   */
public:
  static const std::wstring DEFAULT_STOPWORD_FILE;

private:
  static const std::wstring STOPWORDS_COMMENT;

  /**
   * Returns an unmodifiable instance of the default stop-words set.
   * @return an unmodifiable instance of the default stop-words set.
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
   * Builds an analyzer with the given stop words
   *
   * @param stopwords a stopword set
   * @param stemExclusionSet a stemming exclusion set
   */
public:
  BengaliAnalyzer(std::shared_ptr<CharArraySet> stopwords,
                  std::shared_ptr<CharArraySet> stemExclusionSet);

  /**
   * Builds an analyzer with the given stop words
   *
   * @param stopwords a stopword set
   */
  BengaliAnalyzer(std::shared_ptr<CharArraySet> stopwords);

  /**
   * Builds an analyzer with the default stop words:
   * {@link #DEFAULT_STOPWORD_FILE}.
   */
  BengaliAnalyzer();

  /**
   * Creates
   * {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   * used to tokenize all the text in the provided {@link Reader}.
   *
   * @return {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   *         built from a {@link StandardTokenizer} filtered with
   *         {@link LowerCaseFilter}, {@link DecimalDigitFilter}, {@link
   * IndicNormalizationFilter},
   *         {@link BengaliNormalizationFilter}, {@link SetKeywordMarkerFilter}
   *         if a stem exclusion set is provided, {@link BengaliStemFilter}, and
   *         Bengali Stop words
   */
protected:
  std::shared_ptr<TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<BengaliAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<BengaliAnalyzer>(
        StopwordAnalyzerBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::bn
