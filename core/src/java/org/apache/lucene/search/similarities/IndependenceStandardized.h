#pragma once
#include "stringhelper.h"
#include <cmath>
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
namespace org::apache::lucene::search::similarities
{

/**
 * Standardized measure of distance from independence
 * <p>
 * Described as:
 * "good at tasks that require high recall and high precision, especially
 *  against short queries composed of a few words as in the case of Internet
 *  searches"
 * @lucene.experimental
 */
class IndependenceStandardized : public Independence
{
  GET_CLASS_NAME(IndependenceStandardized)

  /**
   * Sole constructor.
   */
public:
  IndependenceStandardized();

  float score(float freq, float expected) override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<IndependenceStandardized> shared_from_this()
  {
    return std::static_pointer_cast<IndependenceStandardized>(
        Independence::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
