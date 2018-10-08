#pragma once
#include "SimpleNaiveBayesClassifier.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::classification
{
template <typename T>
class ClassificationResult;
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
namespace org::apache::lucene::classification
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A simplistic Lucene based NaiveBayes classifier, with caching feature, see
 * <code>http://en.wikipedia.org/wiki/Naive_Bayes_classifier</code>
 * <p>
 * This is NOT an online classifier.
 *
 * @lucene.experimental
 */
class CachingNaiveBayesClassifier : public SimpleNaiveBayesClassifier
{
  GET_CLASS_NAME(CachingNaiveBayesClassifier)
  // for caching classes this will be the classification class deque
private:
  const std::deque<std::shared_ptr<BytesRef>> cclasses =
      std::deque<std::shared_ptr<BytesRef>>();
  // it's a term-inmap style map_obj, where the inmap contains class-hit pairs to
  // the upper term
  const std::unordered_map<std::wstring,
                           std::unordered_map<std::shared_ptr<BytesRef>, int>>
      termCClassHitCache = std::unordered_map<
          std::wstring, std::unordered_map<std::shared_ptr<BytesRef>, int>>();
  // the term frequency in classes
  const std::unordered_map<std::shared_ptr<BytesRef>, double> classTermFreq =
      std::unordered_map<std::shared_ptr<BytesRef>, double>();
  bool justCachedTerms = false;
  int docsWithClassSize = 0;

  /**
   * Creates a new NaiveBayes classifier with inside caching. If you want less
memory usage you could call
   * {@link #reInitCache(int, bool) reInitCache()}.
GET_CLASS_NAME(ifier)
   *
   * @param indexReader     the reader on the index to be used for
classification
   * @param analyzer       an {@link Analyzer} used to analyze unseen text
GET_CLASS_NAME(ification)
   * @param query          a {@link Query} to eventually filter the docs used
for training the classifier, or {@code null}
   *                       if all the indexed docs should be used
   * @param classFieldName the name of the field used as the output for the
classifier
   * @param textFieldNames the name of the fields used as the inputs for the
classifier
   */
public:
  CachingNaiveBayesClassifier(std::shared_ptr<IndexReader> indexReader,
                              std::shared_ptr<Analyzer> analyzer,
                              std::shared_ptr<Query> query,
                              const std::wstring &classFieldName,
                              std::deque<std::wstring> &textFieldNames);

protected:
  std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
  assignClassNormalizedList(const std::wstring &inputDocument) throw(
      IOException) override;

private:
  std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
  calculateLogLikelihood(std::deque<std::wstring> &tokenizedText) throw(
      IOException);

  std::unordered_map<std::shared_ptr<BytesRef>, int>
  getWordFreqForClassess(const std::wstring &word) ;

  /**
   * This function is building the frame of the cache. The cache is storing the
   * word occurrences to the memory after those searched once. This cache can
   * made 2-100x speedup in proper use, but can eat lot of memory. There is an
   * option to lower the memory consume, if a word have really low occurrence in
   * the index you could filter it out. The other parameter is switching between
   * the term searching, if it true, just the terms in the skeleton will be
   * searched, but if it false the terms whoes not in the cache will be searched
   * out too (but not cached).
   *
   * @param minTermOccurrenceInCache Lower cache size with higher value.
   * @param justCachedTerms          The switch for fully exclude low occurrence
   * docs.
   * @throws IOException If there is a low-level I/O error.
   */
public:
  virtual void reInitCache(int minTermOccurrenceInCache,
                           bool justCachedTerms) ;

protected:
  std::shared_ptr<CachingNaiveBayesClassifier> shared_from_this()
  {
    return std::static_pointer_cast<CachingNaiveBayesClassifier>(
        SimpleNaiveBayesClassifier::shared_from_this());
  }
};

} // namespace org::apache::lucene::classification
