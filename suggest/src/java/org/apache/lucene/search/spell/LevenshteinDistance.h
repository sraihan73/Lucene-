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
 * Levenshtein edit distance class.
 */
class LevenshteinDistance
    : public std::enable_shared_from_this<LevenshteinDistance>,
      public StringDistance
{
  GET_CLASS_NAME(LevenshteinDistance)

  /**
   * Optimized to run a bit faster than the static getDistance().
   * In one benchmark times were 5.3sec using ctr vs 8.5sec w/ static method,
   * thus 37% faster.
   */
public:
  LevenshteinDistance();

  //*****************************
  // Compute Levenshtein distance: see
  // org.apache.commons.lang.StringUtils#getLevenshteinDistance(std::wstring, std::wstring)
  //*****************************
  float getDistance(const std::wstring &target,
                    const std::wstring &other) override;

  virtual int hashCode();

  bool equals(std::any obj) override;

  virtual std::wstring toString();
};

} // namespace org::apache::lucene::search::spell
