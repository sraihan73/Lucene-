#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/AnalyzerWrapper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"

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
namespace org::apache::lucene::analysis::query
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using AnalyzerWrapper = org::apache::lucene::analysis::AnalyzerWrapper;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;

/**
 * An {@link Analyzer} used primarily at query time to wrap another analyzer and
 * provide a layer of protection which prevents very common words from being
 * passed into queries. <p> For very large indexes the cost of reading TermDocs
 * for a very common word can be  high. This analyzer was created after
 * experience with a 38 million doc index which had a term in around 50% of docs
 * and was causing TermQueries for this term to take 2 seconds.
 * </p>
 */
class QueryAutoStopWordAnalyzer final : public AnalyzerWrapper
{
  GET_CLASS_NAME(QueryAutoStopWordAnalyzer)

private:
  const std::shared_ptr<Analyzer> delegate_;
  const std::unordered_map<std::wstring, Set<std::wstring>> stopWordsPerField =
      std::unordered_map<std::wstring, Set<std::wstring>>();
  // The default maximum percentage (40%) of index documents which
  // can contain a term, after which the term is considered to be a stop word.
public:
  static constexpr float defaultMaxDocFreqPercent = 0.4f;

  /**
   * Creates a new QueryAutoStopWordAnalyzer with stopwords calculated for all
   * indexed fields from terms with a document frequency percentage greater than
   * {@link #defaultMaxDocFreqPercent}
   *
   * @param delegate Analyzer whose TokenStream will be filtered
   * @param indexReader IndexReader to identify the stopwords from
   * @throws IOException Can be thrown while reading from the IndexReader
   */
  QueryAutoStopWordAnalyzer(
      std::shared_ptr<Analyzer> delegate_,
      std::shared_ptr<IndexReader> indexReader) ;

  /**
   * Creates a new QueryAutoStopWordAnalyzer with stopwords calculated for all
   * indexed fields from terms with a document frequency greater than the given
   * maxDocFreq
   *
   * @param delegate Analyzer whose TokenStream will be filtered
   * @param indexReader IndexReader to identify the stopwords from
   * @param maxDocFreq Document frequency terms should be above in order to be
   * stopwords
   * @throws IOException Can be thrown while reading from the IndexReader
   */
  QueryAutoStopWordAnalyzer(std::shared_ptr<Analyzer> delegate_,
                            std::shared_ptr<IndexReader> indexReader,
                            int maxDocFreq) ;

  /**
   * Creates a new QueryAutoStopWordAnalyzer with stopwords calculated for all
   * indexed fields from terms with a document frequency percentage greater than
   * the given maxPercentDocs
   *
   * @param delegate Analyzer whose TokenStream will be filtered
   * @param indexReader IndexReader to identify the stopwords from
   * @param maxPercentDocs The maximum percentage (between 0.0 and 1.0) of index
   * documents which contain a term, after which the word is considered to be a
   * stop word
   * @throws IOException Can be thrown while reading from the IndexReader
   */
  QueryAutoStopWordAnalyzer(std::shared_ptr<Analyzer> delegate_,
                            std::shared_ptr<IndexReader> indexReader,
                            float maxPercentDocs) ;

  /**
   * Creates a new QueryAutoStopWordAnalyzer with stopwords calculated for the
   * given selection of fields from terms with a document frequency percentage
   * greater than the given maxPercentDocs
   *
   * @param delegate Analyzer whose TokenStream will be filtered
   * @param indexReader IndexReader to identify the stopwords from
   * @param fields Selection of fields to calculate stopwords for
   * @param maxPercentDocs The maximum percentage (between 0.0 and 1.0) of index
   * documents which contain a term, after which the word is considered to be a
   * stop word
   * @throws IOException Can be thrown while reading from the IndexReader
   */
  QueryAutoStopWordAnalyzer(std::shared_ptr<Analyzer> delegate_,
                            std::shared_ptr<IndexReader> indexReader,
                            std::shared_ptr<std::deque<std::wstring>> fields,
                            float maxPercentDocs) ;

  /**
   * Creates a new QueryAutoStopWordAnalyzer with stopwords calculated for the
   * given selection of fields from terms with a document frequency greater than
   * the given maxDocFreq
   *
   * @param delegate Analyzer whose TokenStream will be filtered
   * @param indexReader IndexReader to identify the stopwords from
   * @param fields Selection of fields to calculate stopwords for
   * @param maxDocFreq Document frequency terms should be above in order to be
   * stopwords
   * @throws IOException Can be thrown while reading from the IndexReader
   */
  QueryAutoStopWordAnalyzer(std::shared_ptr<Analyzer> delegate_,
                            std::shared_ptr<IndexReader> indexReader,
                            std::shared_ptr<std::deque<std::wstring>> fields,
                            int maxDocFreq) ;

protected:
  std::shared_ptr<Analyzer>
  getWrappedAnalyzer(const std::wstring &fieldName) override;

  std::shared_ptr<Analyzer::TokenStreamComponents> wrapComponents(
      const std::wstring &fieldName,
      std::shared_ptr<Analyzer::TokenStreamComponents> components) override;

  /**
   * Provides information on which stop words have been identified for a field
   *
   * @param fieldName The field for which stop words identified in
   * "addStopWords" method calls will be returned
   * @return the stop words identified for a field
   */
public:
  std::deque<std::wstring> getStopWords(const std::wstring &fieldName);

  /**
   * Provides information on which stop words have been identified for all
   * fields
   *
   * @return the stop words (as terms)
   */
  std::deque<std::shared_ptr<Term>> getStopWords();

protected:
  std::shared_ptr<QueryAutoStopWordAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<QueryAutoStopWordAnalyzer>(
        org.apache.lucene.analysis.AnalyzerWrapper::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/query/
