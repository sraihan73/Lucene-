#pragma once
#include "../SimpleNaiveBayesClassifier.h"
#include "DocumentClassifier.h"
#include "stringhelper.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <deque>
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
namespace org::apache::lucene::classification
{
template <typename T>
class ClassificationResult;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::classification::document
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ClassificationResult =
    org::apache::lucene::classification::ClassificationResult;
using SimpleNaiveBayesClassifier =
    org::apache::lucene::classification::SimpleNaiveBayesClassifier;
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A simplistic Lucene based NaiveBayes classifier, see {@code
 * http://en.wikipedia.org/wiki/Naive_Bayes_classifier}
 *
 * @lucene.experimental
 */
class SimpleNaiveBayesDocumentClassifier
    : public SimpleNaiveBayesClassifier,
      public DocumentClassifier<std::shared_ptr<BytesRef>>
{
  GET_CLASS_NAME(SimpleNaiveBayesDocumentClassifier)
  /**
   * {@link org.apache.lucene.analysis.Analyzer} to be used for tokenizing
   * document fields
   */
protected:
  std::unordered_map<std::wstring, std::shared_ptr<Analyzer>> field2analyzer;

  /**
   * Creates a new NaiveBayes classifier.
   *
   * @param indexReader     the reader on the index to be used for
classification
   * @param query          a {@link org.apache.lucene.search.Query} to
eventually filter the docs used for training the classifier, or {@code null}
GET_CLASS_NAME(ification)
   *                       if all the indexed docs should be used
   * @param classFieldName the name of the field used as the output for the
classifier NOTE: must not be havely analyzed
   *                       as the returned class will be a token indexed for
this field
   * @param textFieldNames the name of the fields used as the inputs for the
classifier, they can contain boosting indication e.g. title^10
   */
public:
  SimpleNaiveBayesDocumentClassifier(
      std::shared_ptr<IndexReader> indexReader, std::shared_ptr<Query> query,
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

private:
  std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
  assignNormClasses(std::shared_ptr<Document> inputDocument) ;

  /**
   * This methods performs the analysis for the seed document and extract the
   * boosts if present. This is done only one time for the Seed Document.
   *
   * @param inputDocument         the seed unseen document
   * @param fieldName2tokensArray a map_obj that associated to a field name the deque
   * of token arrays for all its values
   * @param fieldName2boost       a map_obj that associates the boost to the field
   * @throws IOException If there is a low-level I/O error
   */
  void analyzeSeedDocument(
      std::shared_ptr<Document> inputDocument,
      std::unordered_map<std::wstring, std::deque<std::deque<std::wstring>>>
          &fieldName2tokensArray,
      std::unordered_map<std::wstring, float>
          &fieldName2boost) ;

  /**
   * Returns a token array from the {@link
   * org.apache.lucene.analysis.TokenStream} in input
   *
   * @param tokenizedText the tokenized content of a field
   * @return a {@code std::wstring} array of the resulting tokens
   * @throws java.io.IOException If tokenization fails because there is a
   * low-level I/O error
   */
protected:
  virtual std::deque<std::wstring>
  getTokenArray(std::shared_ptr<TokenStream> tokenizedText) ;

  /**
   * @param tokenizedText the tokenized content of a field
   * @param fieldName     the input field name
   * @param term          the {@link Term} referring to the class to calculate
   * the score of
   * @param docsWithClass the total number of docs that have a class
   * @return a normalized score for the class
   * @throws IOException If there is a low-level I/O error
   */
private:
  double calculateLogLikelihood(std::deque<std::wstring> &tokenizedText,
                                const std::wstring &fieldName,
                                std::shared_ptr<Term> term,
                                int docsWithClass) ;

  /**
   * Returns the average number of unique terms times the number of docs
   * belonging to the input class
   *
   * @param  term the class term
   * @return the average number of unique terms
   * @throws java.io.IOException If there is a low-level I/O error
   */
  double
  getTextTermFreqForClass(std::shared_ptr<Term> term,
                          const std::wstring &fieldName) ;

  /**
   * Returns the number of documents of the input class ( from the whole index
   * or from a subset) that contains the word ( in a specific field or in all
   * the fields if no one selected)
   *
   * @param word      the token produced by the analyzer
   * @param fieldName the field the word is coming from
   * @param term      the class term
   * @return number of documents of the input class
   * @throws java.io.IOException If there is a low-level I/O error
   */
  int getWordFreqForClass(const std::wstring &word,
                          const std::wstring &fieldName,
                          std::shared_ptr<Term> term) ;

  double calculateLogPrior(std::shared_ptr<Term> term,
                           int docsWithClassSize) ;

  int docCount(std::shared_ptr<Term> term) ;

protected:
  std::shared_ptr<SimpleNaiveBayesDocumentClassifier> shared_from_this()
  {
    return std::static_pointer_cast<SimpleNaiveBayesDocumentClassifier>(
        org.apache.lucene.classification
            .SimpleNaiveBayesClassifier::shared_from_this());
  }
};

} // namespace org::apache::lucene::classification::document
