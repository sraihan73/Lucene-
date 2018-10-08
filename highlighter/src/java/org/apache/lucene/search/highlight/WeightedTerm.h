#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::search::highlight
{
/** Lightweight class to hold term and a weight value used for scoring this term
 */
class WeightedTerm : public std::enable_shared_from_this<WeightedTerm>
{
  GET_CLASS_NAME(WeightedTerm)
public:
  float weight = 0;  // multiplier
  std::wstring term; // stemmed form
  WeightedTerm(float weight, const std::wstring &term);

  /**
   * @return the term value (stemmed)
   */
  virtual std::wstring getTerm();

  /**
   * @return the weight associated with this term
   */
  virtual float getWeight();

  /**
   * @param term the term value (stemmed)
   */
  virtual void setTerm(const std::wstring &term);

  /**
   * @param weight the weight associated with this term
   */
  virtual void setWeight(float weight);
};

} // namespace org::apache::lucene::search::highlight
