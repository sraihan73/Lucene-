#pragma once
#include "../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "Classifier.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Terms;
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
using Terms = org::apache::lucene::index::Terms;
using Query = org::apache::lucene::search::Query;
using FST = org::apache::lucene::util::fst::FST;

/**
 * A perceptron (see <code>http://en.wikipedia.org/wiki/Perceptron</code>) based
 * <code>Boolean</code> {@link org.apache.lucene.classification.Classifier}. The
 * weights are calculated using
 * {@link org.apache.lucene.index.TermsEnum#totalTermFreq} both on a per field
 * and a per document basis and then a corresponding
 * {@link org.apache.lucene.util.fst.FST} is used for class assignment.
 *
 * @lucene.experimental
 */
class BooleanPerceptronClassifier
    : public std::enable_shared_from_this<BooleanPerceptronClassifier>,
      public Classifier<bool>
{
  GET_CLASS_NAME(BooleanPerceptronClassifier)

private:
  const std::optional<double> bias;
  const std::shared_ptr<Terms> textTerms;
  const std::shared_ptr<Analyzer> analyzer;
  const std::wstring textFieldName;
  std::shared_ptr<FST<int64_t>> fst;

  /**
   * Creates a {@link BooleanPerceptronClassifier}
   *
   * @param indexReader     the reader on the index to be used for
classification
   * @param analyzer       an {@link Analyzer} used to analyze unseen text
GET_CLASS_NAME(ification)
   * @param query          a {@link Query} to eventually filter the docs used
for training the classifier, or {@code null}
   *                       if all the indexed docs should be used
   * @param batchSize      the size of the batch of docs to use for updating the
perceptron weights
   * @param bias      the bias used for class separation
   * @param classFieldName the name of the field used as the output for the
classifier
   * @param textFieldName  the name of the field used as input for the
classifier
   * @throws IOException if the building of the underlying {@link FST} fails and
/ or {@link TermsEnum} for the text field GET_CLASS_NAME(ifier)
   *                     cannot be found
   */
public:
  BooleanPerceptronClassifier(
      std::shared_ptr<IndexReader> indexReader,
      std::shared_ptr<Analyzer> analyzer, std::shared_ptr<Query> query,
      std::optional<int> &batchSize, std::optional<double> &bias,
      const std::wstring &classFieldName,
      const std::wstring &textFieldName) ;

private:
  void updateWeights(std::shared_ptr<IndexReader> indexReader, int docId,
                     std::optional<bool> &assignedClass,
                     std::shared_ptr<SortedMap<std::wstring, double>> weights,
                     double modifier, bool updateFST) ;

  void
  updateFST(std::shared_ptr<SortedMap<std::wstring, double>> weights) throw(
      IOException);

public:
  std::shared_ptr<ClassificationResult<bool>>
  assignClass(const std::wstring &text)  override;

  std::deque<ClassificationResult<bool>>
  getClasses(const std::wstring &text)  override;

  std::deque<ClassificationResult<bool>>
  getClasses(const std::wstring &text, int max)  override;
};

} // namespace org::apache::lucene::classification
