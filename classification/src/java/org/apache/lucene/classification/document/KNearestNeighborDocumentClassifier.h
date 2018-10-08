#pragma once
#include "../KNearestNeighborClassifier.h"
#include "DocumentClassifier.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
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
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::classification
{
template <typename T>
class ClassificationResult;
}
namespace org::apache::lucene::document
{
class Document;
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
namespace org::apache::lucene::classification::document
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using ClassificationResult =
    org::apache::lucene::classification::ClassificationResult;
using KNearestNeighborClassifier =
    org::apache::lucene::classification::KNearestNeighborClassifier;
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using Query = org::apache::lucene::search::Query;
using TopDocs = org::apache::lucene::search::TopDocs;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A k-Nearest Neighbor Document classifier (see
<code>http://en.wikipedia.org/wiki/K-nearest_neighbors</code>) based
 * on {@link org.apache.lucene.queries.mlt.MoreLikeThis} .
GET_CLASS_NAME(ifier)
 *
 * @lucene.experimental
 */
class KNearestNeighborDocumentClassifier
    : public KNearestNeighborClassifier,
      public DocumentClassifier<std::shared_ptr<BytesRef>>
{
  GET_CLASS_NAME(KNearestNeighborDocumentClassifier)

  /**
   * map_obj of per field analyzers
   */
protected:
  std::unordered_map<std::wstring, std::shared_ptr<Analyzer>> field2analyzer;

  /**
   * Creates a {@link KNearestNeighborClassifier}.
   *
   * @param indexReader     the reader on the index to be used for
classification
   * @param similarity     the {@link Similarity} to be used by the underlying
{@link IndexSearcher} or {@code null} GET_CLASS_NAME(ification)
   *                       (defaults to {@link
org.apache.lucene.search.similarities.BM25Similarity})
   * @param query          a {@link org.apache.lucene.search.Query} to
eventually filter the docs used for training the classifier, or {@code null}
   *                       if all the indexed docs should be used
   * @param k              the no. of docs to select in the MLT results to find
the nearest neighbor
   * @param minDocsFreq    {@link
org.apache.lucene.queries.mlt.MoreLikeThis#minDocFreq} parameter
   * @param minTermFreq    {@link
org.apache.lucene.queries.mlt.MoreLikeThis#minTermFreq} parameter
   * @param classFieldName the name of the field used as the output for the
classifier
   * @param field2analyzer map_obj with key a field name and the related
{org.apache.lucene.analysis.Analyzer} GET_CLASS_NAME(FieldName)
   * @param textFieldNames the name of the fields used as the inputs for the
classifier, they can contain boosting indication e.g. title^10
   */
public:
  KNearestNeighborDocumentClassifier(
      std::shared_ptr<IndexReader> indexReader,
      std::shared_ptr<Similarity> similarity, std::shared_ptr<Query> query,
      int k, int minDocsFreq, int minTermFreq,
      const std::wstring &classFieldName,
      std::unordered_map<std::wstring, std::shared_ptr<Analyzer>>
          &field2analyzer,
      std::deque<std::wstring> &textFieldNames);

  std::shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>>
  assignClass(std::shared_ptr<Document> document)  override;

  std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
  getClasses(std::shared_ptr<Document> document)  override;

  std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
  getClasses(std::shared_ptr<Document> document,
             int max)  override;

  /**
   * Returns the top k results from a More Like This query based on the input
   * document
   *
   * @param document the document to use for More Like This search
   * @return the top results for the MLT query
   * @throws IOException If there is a low-level I/O error
   */
private:
  std::shared_ptr<TopDocs>
  knnSearch(std::shared_ptr<Document> document) ;

protected:
  std::shared_ptr<KNearestNeighborDocumentClassifier> shared_from_this()
  {
    return std::static_pointer_cast<KNearestNeighborDocumentClassifier>(
        org.apache.lucene.classification
            .KNearestNeighborClassifier::shared_from_this());
  }
};

} // namespace org::apache::lucene::classification::document
