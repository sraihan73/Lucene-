#pragma once
#include "../ClassificationResult.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Document.h"

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
namespace org::apache::lucene::classification::document
{

using ClassificationResult =
    org::apache::lucene::classification::ClassificationResult;
using Document = org::apache::lucene::document::Document;

/**
 * A classifier, see
<code>http://en.wikipedia.org/wiki/Classifier_(mathematics)</code>, which assign
classes of type
 * <code>T</code> to a {@link org.apache.lucene.document.Document}s
GET_CLASS_NAME(ifier,)
 *
 * @lucene.experimental
 */
template <typename T>
class DocumentClassifier
{
  GET_CLASS_NAME(DocumentClassifier)
  /**
   * Assign a class (with score) to the given {@link
org.apache.lucene.document.Document}
   *
   * @param document a {@link org.apache.lucene.document.Document}  to be
classified. Fields are considered features for the classification.
   * @return a {@link org.apache.lucene.classification.ClassificationResult}
holding assigned class of type <code>T</code> and score GET_CLASS_NAME(ified.)
   * @throws java.io.IOException If there is a low-level I/O error.
   */
public:
  virtual std::shared_ptr<ClassificationResult<T>>
  assignClass(std::shared_ptr<Document> document) = 0;

  /**
   * Get all the classes (sorted by score, descending) assigned to the given
{@link org.apache.lucene.document.Document}.
   *
   * @param document a {@link org.apache.lucene.document.Document}  to be
classified. Fields are considered features for the classification.
   * @return the whole deque of {@link
org.apache.lucene.classification.ClassificationResult}, the classes and scores.
Returns <code>null</code> if the classifier can't make lists.
GET_CLASS_NAME(ified.)
   * @throws java.io.IOException If there is a low-level I/O error.
   */
  virtual std::deque<ClassificationResult<T>>
  getClasses(std::shared_ptr<Document> document) = 0;

  /**
   * Get the first <code>max</code> classes (sorted by score, descending)
   * assigned to the given text std::wstring.
   *
   * @param document a {@link org.apache.lucene.document.Document}  to be
   * classified. Fields are considered features for the classification.
   * @param max      the number of return deque elements
   * @return the whole deque of {@link
   * org.apache.lucene.classification.ClassificationResult}, the classes and
   * scores. Cut for "max" number of elements. Returns <code>null</code> if the
   * classifier can't make lists.
   * @throws java.io.IOException If there is a low-level I/O error.
   */
  virtual std::deque<ClassificationResult<T>>
  getClasses(std::shared_ptr<Document> document, int max) = 0;
};
} // #include  "core/src/java/org/apache/lucene/classification/document/
