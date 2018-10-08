#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::classification
{
template <typename T>
class Classifier;
}

#include  "core/src/java/org/apache/lucene/classification/utils/ConfusionMatrix.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

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
namespace org::apache::lucene::classification::utils
{

using Classifier = org::apache::lucene::classification::Classifier;
using IndexReader = org::apache::lucene::index::IndexReader;

/**
 * Utility class to generate the confusion matrix of a {@link Classifier}
 */
class ConfusionMatrixGenerator
    : public std::enable_shared_from_this<ConfusionMatrixGenerator>
{
  GET_CLASS_NAME(ConfusionMatrixGenerator)

private:
  ConfusionMatrixGenerator();

  /**
   * get the {@link
org.apache.lucene.classification.utils.ConfusionMatrixGenerator.ConfusionMatrix}
of a given {@link Classifier},
   * generated on the given {@link IndexReader}, class and text fields.
GET_CLASS_NAME(ification.utils.ConfusionMatrixGenerator.ConfusionMatrix})
   *
   * @param reader              the {@link IndexReader} containing the index
used for creating the {@link Classifier}
   * @param classifier          the {@link Classifier} whose confusion matrix
has to be generated
   * @param classFieldName      the name of the Lucene field used as the
classifier's output
   * @param textFieldName       the nome the Lucene field used as the
classifier's input
   * @param timeoutMilliseconds timeout to wait before stopping creating the
confusion matrix
   * @param <T>                 the return type of the {@link
ClassificationResult} returned by the given {@link Classifier}
   * @return a {@link
org.apache.lucene.classification.utils.ConfusionMatrixGenerator.ConfusionMatrix}
   * @throws IOException if problems occurr while reading the index or using the
classifier
   */
public:
  template <typename T>
  static std::shared_ptr<ConfusionMatrix>
  getConfusionMatrix(std::shared_ptr<IndexReader> reader,
                     std::shared_ptr<Classifier<T>> classifier,
                     const std::wstring &classFieldName,
                     const std::wstring &textFieldName,
                     int64_t timeoutMilliseconds) ;

  /**
   * a confusion matrix, backed by a {@link Map} representing the linearized
   * matrix
   */
public:
  class ConfusionMatrix : public std::enable_shared_from_this<ConfusionMatrix>
  {
    GET_CLASS_NAME(ConfusionMatrix)

  private:
    const std::unordered_map<std::wstring,
                             std::unordered_map<std::wstring, int64_t>>
        linearizedMatrix;
    const double avgClassificationTime;
    const int numberOfEvaluatedDocs;
    double accuracy = -1;

    ConfusionMatrix(
        std::unordered_map<std::wstring,
                           std::unordered_map<std::wstring, int64_t>>
            &linearizedMatrix,
        double avgClassificationTime, int numberOfEvaluatedDocs);

    /**
     * get the linearized confusion matrix as a {@link Map}
     *
     * @return a {@link Map} whose keys are the correct classification answers
     * and whose values are the actual answers' counts
     */
  public:
    virtual std::unordered_map<std::wstring,
                               std::unordered_map<std::wstring, int64_t>>
    getLinearizedMatrix();

    /**
     * calculate precision on the given class
     *
     * @param klass the class to calculate the precision for
     * @return the precision for the given class
     */
    virtual double getPrecision(const std::wstring &klass);

    /**
     * calculate recall on the given class
     *
     * @param klass the class to calculate the recall for
     * @return the recall for the given class
     */
    virtual double getRecall(const std::wstring &klass);

    /**
     * get the F-1 measure of the given class
     *
     * @param klass the class to calculate the F-1 measure for
     * @return the F-1 measure for the given class
     */
    virtual double getF1Measure(const std::wstring &klass);

    /**
     * get the F-1 measure on this confusion matrix
     *
     * @return the F-1 measure
     */
    virtual double getF1Measure();

    /**
     * Calculate accuracy on this confusion matrix using the formula:
     * {@literal accuracy = correctly-classified / (correctly-classified +
     * wrongly-classified)}
     *
     * @return the accuracy
     */
    virtual double getAccuracy();

    /**
     * get the macro averaged precision (see {@link #getPrecision(std::wstring)}) over
     * all the classes.
     *
     * @return the macro averaged precision as computed from the confusion
     * matrix
     */
    virtual double getPrecision();

    /**
     * get the macro averaged recall (see {@link #getRecall(std::wstring)}) over all
     * the classes
     *
     * @return the recall as computed from the confusion matrix
     */
    virtual double getRecall();

    virtual std::wstring toString();

    /**
     * get the average classification time in milliseconds
     *
     * @return the avg classification time
     */
    virtual double getAvgClassificationTime();

    /**
     * get the no. of documents evaluated while generating this confusion matrix
     *
     * @return the no. of documents evaluated
     */
    virtual int getNumberOfEvaluatedDocs();
  };
};

} // #include  "core/src/java/org/apache/lucene/classification/utils/
