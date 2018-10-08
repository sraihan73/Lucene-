#pragma once
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
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
namespace org::apache::lucene::analysis::cn::smart
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * <p>
 * SmartChineseAnalyzer is an analyzer for Chinese or mixed Chinese-English
 * text. The analyzer uses probabilistic knowledge to find the optimal word
 * segmentation for Simplified Chinese text. The text is first broken into
 * sentences, then each sentence is segmented into words.
 * </p>
 * <p>
 * Segmentation is based upon the <a
 * href="http://en.wikipedia.org/wiki/Hidden_Markov_Model">Hidden Markov
 * Model</a>. A large training corpus was used to calculate Chinese word
 * frequency probability.
 * </p>
 * <p>
 * This analyzer requires a dictionary to provide statistical data.
 * SmartChineseAnalyzer has an included dictionary out-of-box.
 * </p>
 * <p>
 * The included dictionary data is from <a
 * href="http://www.ictclas.org">ICTCLAS1.0</a>. Thanks to ICTCLAS for their
 * hard work, and for contributing the data under GPLv3 2 License!
 * </p>
 * @lucene.experimental
 */
class SmartChineseAnalyzer final : public Analyzer
{
  GET_CLASS_NAME(SmartChineseAnalyzer)

private:
  const std::shared_ptr<CharArraySet> stopWords;

  static const std::wstring DEFAULT_STOPWORD_FILE;

  static const std::wstring STOPWORD_FILE_COMMENT;

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

  public:
    static std::shared_ptr<CharArraySet>
    loadDefaultStopWordSet() ;
  };

  /**
   * Create a new SmartChineseAnalyzer, using the default stopword deque.
   */
public:
  SmartChineseAnalyzer();

  /**
   * <p>
   * Create a new SmartChineseAnalyzer, optionally using the default stopword
   * deque.
   * </p>
   * <p>
   * The included default stopword deque is simply a deque of punctuation.
   * If you do not use this deque, punctuation will not be removed from the text!
   * </p>
   *
   * @param useDefaultStopWords true to use the default stopword deque.
   */
  SmartChineseAnalyzer(bool useDefaultStopWords);

  /**
   * <p>
   * Create a new SmartChineseAnalyzer, using the provided {@link Set} of
   * stopwords.
   * </p>
   * <p>
   * Note: the set should include punctuation, unless you want to index
   * punctuation!
   * </p>
   * @param stopWords {@link Set} of stopwords to use.
   */
  SmartChineseAnalyzer(std::shared_ptr<CharArraySet> stopWords);

  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

protected:
  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<SmartChineseAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<SmartChineseAnalyzer>(
        org.apache.lucene.analysis.Analyzer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/cn/smart/
