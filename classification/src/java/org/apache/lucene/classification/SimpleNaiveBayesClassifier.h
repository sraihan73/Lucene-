#pragma once
#include "Classifier.h"
#include "stringhelper.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <deque>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
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
namespace org::apache::lucene::index
{
class Term;
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
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A simplistic Lucene based NaiveBayes classifier, see
 * <code>http://en.wikipedia.org/wiki/Naive_Bayes_classifier</code>
 *
 * @lucene.experimental
 */
class SimpleNaiveBayesClassifier
    : public std::enable_shared_from_this<SimpleNaiveBayesClassifier>,
      public Classifier<std::shared_ptr<BytesRef>>
{
  GET_CLASS_NAME(SimpleNaiveBayesClassifier)

  /**
   * {@link org.apache.lucene.index.IndexReader} used to access the {@link
   * org.apache.lucene.classification.Classifier}'s index
   */
protected:
  const std::shared_ptr<IndexReader> indexReader;

  /**
   * names of the fields to be used as input text
   */
  std::deque<std::wstring> const textFieldNames;

  /**
   * name of the field to be used as a class / category output
   */
  const std::wstring classFieldName;

  /**
   * {@link org.apache.lucene.analysis.Analyzer} to be used for tokenizing
   * unseen input text
   */
  const std::shared_ptr<Analyzer> analyzer;

  /**
   * {@link org.apache.lucene.search.IndexSearcher} to run searches on the index
   * for retrieving frequencies
   */
  const std::shared_ptr<IndexSearcher> indexSearcher;

  /**
   * {@link org.apache.lucene.search.Query} used to eventually filter the
   * document set to be used to classify
   */
  const std::shared_ptr<Query> query;

  /**
   * Creates a new NaiveBayes classifier.
   *
   * @param indexReader     the reader on the index to be used for
classification
   * @param analyzer       an {@link Analyzer} used to analyze unseen text
GET_CLASS_NAME(ification)
   * @param query          a {@link Query} to eventually filter the docs used
for training the classifier, or {@code null}
   *                       if all the indexed docs should be used
   * @param classFieldName the name of the field used as the output for the
classifier NOTE: must not be havely analyzed
   *                       as the returned class will be a token indexed for
this field
   * @param textFieldNames the name of the fields used as the inputs for the
classifier, NO boosting supported per field
   */
public:
  SimpleNaiveBayesClassifier(std::shared_ptr<IndexReader> indexReader,
                             std::shared_ptr<Analyzer> analyzer,
                             std::shared_ptr<Query> query,
                             const std::wstring &classFieldName,
                             std::deque<std::wstring> &textFieldNames);

  std::shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>>
  assignClass(const std::wstring &inputDocument)  override;

  std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
  getClasses(const std::wstring &text)  override;

  std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
  getClasses(const std::wstring &text, int max)  override;

  /**
   * Calculate probabilities for all classes for a given input text
   * @param inputDocument the input text as a {@code std::wstring}
GET_CLASS_NAME(es)
   * @return a {@code List} of {@code ClassificationResult}, one for each
existing class
   * @throws IOException if assigning probabilities fails
   */
protected:
  virtual std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
  assignClassNormalizedList(const std::wstring &inputDocument) throw(
      IOException);

  /**
   * count the number of documents in the index having at least a value for the
   * 'class' field
   *
   * @return the no. of documents having a value for the 'class' field
   * @throws IOException if accessing to term vectors or search fails
   */
  virtual int countDocsWithClass() ;

  /**
   * tokenize a <code>std::wstring</code> on this classifier's text fields and
   * analyzer
   *
   * @param text the <code>std::wstring</code> representing an input text (to be
   * classified)
   * @return a <code>std::wstring</code> array of the resulting tokens
   * @throws IOException if tokenization fails
   */
  virtual std::deque<std::wstring>
  tokenize(const std::wstring &text) ;

private:
  double calculateLogLikelihood(std::deque<std::wstring> &tokenizedText,
                                std::shared_ptr<Term> term,
                                int docsWithClass) ;

  /**
   * Returns the average number of unique terms times the number of docs
   * belonging to the input class
   * @param term the term representing the class
   * @return the average number of unique terms
   * @throws IOException if a low level I/O problem happens
   */
  double getTextTermFreqForClass(std::shared_ptr<Term> term) ;

  /**
   * Returns the number of documents of the input class ( from the whole index
   * or from a subset) that contains the word ( in a specific field or in all
   * the fields if no one selected)
   * @param word the token produced by the analyzer
   * @param term the term representing the class
   * @return the number of documents of the input class
   * @throws IOException if a low level I/O problem happens
   */
  int getWordFreqForClass(const std::wstring &word,
                          std::shared_ptr<Term> term) ;

  double calculateLogPrior(std::shared_ptr<Term> term,
                           int docsWithClassSize) ;

  int docCount(std::shared_ptr<Term> term) ;

  /**
   * Normalize the classification results based on the max score available
   * @param assignedClasses the deque of assigned classes
   * @return the normalized results
   */
protected:
  virtual std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
  normClassificationResults(
      std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
          &assignedClasses);
};

} // namespace org::apache::lucene::classification
