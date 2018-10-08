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
namespace org::apache::lucene::search::similarities
{

/**
 * Computes the measure of divergence from independence for DFI
 * scoring functions.
 * <p>
 * See http://trec.nist.gov/pubs/trec21/papers/irra.web.nb.pdf for more
 * information on different methods.
 * @lucene.experimental
 */
class Independence : public std::enable_shared_from_this<Independence>
{
  GET_CLASS_NAME(Independence)

  /**
   * Sole constructor. (For invocation by subclass
   * constructors, typically implicit.)
   */
public:
  Independence();

  /**
   * Computes distance from independence
   * @param freq actual term frequency
   * @param expected expected term frequency
   */
  virtual float score(float freq, float expected) = 0;

  // subclasses must provide a name
  std::wstring toString() = 0;
  override
};

} // namespace org::apache::lucene::search::similarities
