#pragma once
#include "stringhelper.h"
#include <memory>

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

/**
 * The result of a call to {@link Classifier#assignClass(std::wstring)} holding an
 * assigned class of type <code>T</code> and a score.
 *
 * @lucene.experimental
 */
template <typename T>
class ClassificationResult
    : public std::enable_shared_from_this<ClassificationResult>,
      public Comparable<ClassificationResult<T>>
{
  GET_CLASS_NAME(ClassificationResult)

private:
  const T assignedClass;
  const double score;

  /**
   * Constructor
   *
   * @param assignedClass the class <code>T</code> assigned by a {@link
   * Classifier}
   * @param score         the score for the assignedClass as a
   * <code>double</code>
   */
public:
  ClassificationResult(T assignedClass, double score)
      : assignedClass(assignedClass), score(score)
  {
  }

  /**
   * retrieve the result class
   *
   * @return a <code>T</code> representing an assigned class
   */
  virtual T getAssignedClass() { return assignedClass; }

  /**
   * retrieve the result score
   *
   * @return a <code>double</code> representing a result score
   */
  virtual double getScore() { return score; }

  int compareTo(std::shared_ptr<ClassificationResult<T>> o) override
  {
    return this->getScore() < o->getScore()
               ? 1
               : this->getScore() > o->getScore() ? -1 : 0;
  }
};

} // #include  "core/src/java/org/apache/lucene/classification/
