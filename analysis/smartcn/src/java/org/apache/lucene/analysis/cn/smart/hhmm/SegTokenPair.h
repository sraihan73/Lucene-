#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
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
namespace org::apache::lucene::analysis::cn::smart::hhmm
{

/**
 * A pair of tokens in {@link SegGraph}
 * @lucene.experimental
 */
class SegTokenPair : public std::enable_shared_from_this<SegTokenPair>
{
  GET_CLASS_NAME(SegTokenPair)

public:
  std::deque<wchar_t> charArray;

  /**
   * index of the first token in {@link SegGraph}
   */
  int from = 0;

  /**
   * index of the second token in {@link SegGraph}
   */
  int to = 0;

  double weight = 0;

  SegTokenPair(std::deque<wchar_t> &idArray, int from, int to, double weight);

  /**
   * @see java.lang.Object#hashCode()
   */
  virtual int hashCode();

  /**
   * @see java.lang.Object#equals(java.lang.Object)
   */
  bool equals(std::any obj) override;
};

} // namespace org::apache::lucene::analysis::cn::smart::hhmm
