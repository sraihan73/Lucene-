#pragma once
#include "Classifier.h"
#include "stringhelper.h"
#include <algorithm>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::classification
{
template <typename T>
class ClassificationResult;
}
namespace org::apache::lucene::search
{
class TopDocs;
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
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TopDocs = org::apache::lucene::search::TopDocs;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A k-Nearest Neighbor classifier based on {@link NearestFuzzyQuery}.
 *
 * @lucene.experimental
 */
class KNearestFuzzyClassifier
    : public std::enable_shared_from_this<KNearestFuzzyClassifier>,
      public Classifier<std::shared_ptr<BytesRef>>
{
  GET_CLASS_NAME(KNearestFuzzyClassifier)

  /**
   * the name of the fields used as the input text
   */
private:
  std::deque<std::wstring> const textFieldNames;

  /**
   * the name of the field used as the output text
   */
  const std::wstring classFieldName;

  /**
   * an {@link IndexSearcher} used to perform queries
   */
  const std::shared_ptr<IndexSearcher> indexSearcher;

  /**
   * the no. of docs to compare in order to find the nearest neighbor to the
   * input text
   */
  const int k;

  /**
   * a {@link Query} used to filter the documents that should be used from this
   * classifier's underlying {@link LeafReader}
   */
  const std::shared_ptr<Query> query;
  const std::shared_ptr<Analyzer> analyzer;

  /**
   * Creates a {@link KNearestFuzzyClassifier}.
   *
   * @param indexReader    the reader on the index to be used for classification
   * @param analyzer       an {@link Analyzer} used to analyze unseen text
GET_CLASS_NAME(ification)
   * @param similarity     the {@link Similarity} to be used by the underlying
{@link IndexSearcher} or {@code null}
   *                       (defaults to {@link BM25Similarity})
   * @param query          a {@link Query} to eventually filter the docs used
for training the classifier, or {@code null}
   *                       if all the indexed docs should be used
   * @param k              the no. of docs to select in the MLT results to find
the nearest neighbor
   * @param classFieldName the name of the field used as the output for the
classifier
   * @param textFieldNames the name of the fields used as the inputs for the
classifier, they can contain boosting indication e.g. title^10
   */
public:
  KNearestFuzzyClassifier(std::shared_ptr<IndexReader> indexReader,
                          std::shared_ptr<Similarity> similarity,
                          std::shared_ptr<Analyzer> analyzer,
                          std::shared_ptr<Query> query, int k,
                          const std::wstring &classFieldName,
                          std::deque<std::wstring> &textFieldNames);

  std::shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>>
  assignClass(const std::wstring &text)  override;

  std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
  getClasses(const std::wstring &text)  override;

  std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
  getClasses(const std::wstring &text, int max)  override;

private:
  std::shared_ptr<TopDocs>
  knnSearch(const std::wstring &text) ;

  /**
   * build a deque of classification results from search results
   *
   * @param topDocs the search results as a {@link TopDocs} object
   * @return a {@link List} of {@link ClassificationResult}, one for each
   * existing class
   * @throws IOException if it's not possible to get the stored value of class
   * field
   */
  std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
  buildListFromTopDocs(std::shared_ptr<TopDocs> topDocs) ;

public:
  virtual std::wstring toString();
};

} // namespace org::apache::lucene::classification
