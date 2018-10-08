#pragma once
#include "ClassificationResult.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
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

/**
 * A classifier, see
 * <code>http://en.wikipedia.org/wiki/Classifier_(mathematics)</code>, which
 * assign classes of type <code>T</code>
 *
 * @lucene.experimental
 */
template <typename T>
class Classifier
{
  GET_CLASS_NAME(Classifier)

  /**
   * Assign a class (with score) to the given text std::wstring
   *
   * @param text a std::wstring containing text to be classified
   * @return a {@link ClassificationResult} holding assigned class of type
<code>T</code> and score GET_CLASS_NAME(ified)
   * @throws IOException If there is a low-level I/O error.
   */
public:
  virtual std::shared_ptr<ClassificationResult<T>>
  assignClass(const std::wstring &text) = 0;

  /**
   * Get all the classes (sorted by score, descending) assigned to the given
text std::wstring.
   *
   * @param text a std::wstring containing text to be classified
   * @return the whole deque of {@link ClassificationResult}, the classes and
scores. Returns <code>null</code> if the classifier can't make lists.
GET_CLASS_NAME(ified)
   * @throws IOException If there is a low-level I/O error.
   */
  virtual std::deque<ClassificationResult<T>>
  getClasses(const std::wstring &text) = 0;

  /**
   * Get the first <code>max</code> classes (sorted by score, descending)
   * assigned to the given text std::wstring.
   *
   * @param text a std::wstring containing text to be classified
   * @param max  the number of return deque elements
   * @return the whole deque of {@link ClassificationResult}, the classes and
   * scores. Cut for "max" number of elements. Returns <code>null</code> if the
   * classifier can't make lists.
   * @throws IOException If there is a low-level I/O error.
   */
  virtual std::deque<ClassificationResult<T>>
  getClasses(const std::wstring &text, int max) = 0;
};

} // namespace org::apache::lucene::classification
