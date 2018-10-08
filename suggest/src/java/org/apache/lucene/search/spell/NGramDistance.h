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
 * N-Gram version of edit distance based on paper by Grzegorz Kondrak,
 * "N-gram similarity and distance". Proceedings of the Twelfth International
 * Conference on std::wstring Processing and Information Retrieval (SPIRE 2005), pp.
 * 115-126, Buenos Aires, Argentina, November 2005.
 * http://www.cs.ualberta.ca/~kondrak/papers/spire05.pdf
 *
 * This implementation uses the position-based optimization to compute partial
 * matches of n-gram sub-strings and adds a null-character prefix of size n-1
 * so that the first character is contained in the same number of n-grams as
 * a middle character.  Null-character prefix matches are discounted so that
 * strings with no matching characters will return a distance of 0.
 *
 */
class NGramDistance : public std::enable_shared_from_this<NGramDistance>,
                      public StringDistance
{
  GET_CLASS_NAME(NGramDistance)

private:
  int n = 0;

  /**
   * Creates an N-Gram distance measure using n-grams of the specified size.
   * @param size The size of the n-gram to be used to compute the string
   * distance.
   */
public:
  NGramDistance(int size);

  /**
   * Creates an N-Gram distance measure using n-grams of size 2.
   */
  NGramDistance();

  float getDistance(const std::wstring &source,
                    const std::wstring &target) override;

  virtual int hashCode();

  bool equals(std::any obj) override;

  virtual std::wstring toString();
};

} // namespace org::apache::lucene::search::spell
