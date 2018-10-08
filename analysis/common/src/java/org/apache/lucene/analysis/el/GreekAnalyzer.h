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
namespace org::apache::lucene::analysis::el
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * {@link Analyzer} for the Greek language.
 * <p>
 * Supports an external deque of stopwords (words
 * that will not be indexed at all).
 * A default set of stopwords is used unless an alternative deque is specified.
 * </p>
 *
 * <p><b>NOTE</b>: This class uses the same {@link
org.apache.lucene.util.Version}
 * dependent settings as {@link StandardAnalyzer}.</p>
GET_CLASS_NAME(uses)
 */
class GreekAnalyzer final : public StopwordAnalyzerBase
{
  GET_CLASS_NAME(GreekAnalyzer)
  /** File containing default Greek stopwords. */
public:
  static const std::wstring DEFAULT_STOPWORD_FILE;

  /**
   * Returns a set of default Greek-stopwords
   * @return a set of default Greek-stopwords
   */
  static std::shared_ptr<CharArraySet> getDefaultStopSet();

private:
  class DefaultSetHolder : public std::enable_shared_from_this<DefaultSetHolder>
  {
    GET_CLASS_NAME(DefaultSetHolder)
  private:
    static const std::shared_ptr<CharArraySet> DEFAULT_SET;

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
   * Builds an analyzer with the default stop words.
   */
public:
  GreekAnalyzer();

  /**
   * Builds an analyzer with the given stop words.
   * <p>
   * <b>NOTE:</b> The stopwords set should be pre-processed with the logic of
   * {@link GreekLowerCaseFilter} for best results.
   *
   * @param stopwords a stopword set
   */
  GreekAnalyzer(std::shared_ptr<CharArraySet> stopwords);

  /**
   * Creates
   * {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   * used to tokenize all the text in the provided {@link Reader}.
   *
   * @return {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   *         built from a {@link StandardTokenizer} filtered with
   *         {@link GreekLowerCaseFilter}, {@link StandardFilter},
   *         {@link StopFilter}, and {@link GreekStemFilter}
   */
protected:
  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<GreekAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<GreekAnalyzer>(
        org.apache.lucene.analysis.StopwordAnalyzerBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::el
