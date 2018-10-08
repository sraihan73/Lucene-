#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

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
namespace org::apache::lucene::search::spell
{

/**
 * Similarity measure for short strings such as person names.
 * <p>
 * @see <a
 * href="http://en.wikipedia.org/wiki/Jaro%E2%80%93Winkler_distance">http://en.wikipedia.org/wiki/Jaro%E2%80%93Winkler_distance</a>
 */
class JaroWinklerDistance
    : public std::enable_shared_from_this<JaroWinklerDistance>,
      public StringDistance
{
  GET_CLASS_NAME(JaroWinklerDistance)

private:
  float threshold = 0.7f;

  /**
   * Creates a new distance metric with the default threshold
   * for the Jaro Winkler bonus (0.7)
   * @see #setThreshold(float)
   */
public:
  JaroWinklerDistance();

private:
  std::deque<int> matches(const std::wstring &s1, const std::wstring &s2);

public:
  float getDistance(const std::wstring &s1, const std::wstring &s2) override;

  /**
   * Sets the threshold used to determine when Winkler bonus should be used.
   * Set to a negative value to get the Jaro distance.
   * @param threshold the new value of the threshold
   */
  virtual void setThreshold(float threshold);

  /**
   * Returns the current value of the threshold used for adding the Winkler
   * bonus. The default value is 0.7.
   * @return the current value of the threshold
   */
  virtual float getThreshold();

  virtual int hashCode();

  bool equals(std::any obj) override;

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/search/spell/
