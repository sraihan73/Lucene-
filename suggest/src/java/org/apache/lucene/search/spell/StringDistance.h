#pragma once
#include "stringhelper.h"
#include <string>

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
 * Interface for string distances.
 */
class StringDistance
{
  GET_CLASS_NAME(StringDistance)

  /**
   * Returns a float between 0 and 1 based on how similar the specified strings
   * are to one another. Returning a value of 1 means the specified strings are
   * identical and 0 means the string are maximally different.
   * @param s1 The first string.
   * @param s2 The second string.
   * @return a float between 0 and 1 based on how similar the specified strings
   * are to one another.
   */
public:
  virtual float getDistance(const std::wstring &s1, const std::wstring &s2) = 0;
};

} // #include  "core/src/java/org/apache/lucene/search/spell/
