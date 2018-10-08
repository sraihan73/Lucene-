#pragma once
#include "stringhelper.h"
#include <memory>

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
namespace org::apache::lucene::util
{

/**
 * Interface for Bitset-like structures.
 * @lucene.experimental
 */

class Bits
{
  GET_CLASS_NAME(Bits)
  /**
   * Returns the value of the bit with the specified <code>index</code>.
   * @param index index, should be non-negative and &lt; {@link #length()}.
   *        The result of passing negative or out of bounds values is undefined
   *        by this interface, <b>just don't do it!</b>
   * @return <code>true</code> if the bit is set, <code>false</code> otherwise.
   */
public:
  virtual bool get(int index) = 0;

  /** Returns the number of bits in this set */
  virtual int length() = 0;

  static std::deque<std::shared_ptr<Bits>> const EMPTY_ARRAY;
};
  /**
   * Bits impl of the specified length with all bits set.
   */
  class MatchAllBits : public std::enable_shared_from_this<MatchAllBits>,
                       public Bits
  {
    GET_CLASS_NAME(MatchAllBits)
  public:
    const int len;

    MatchAllBits(int len);

    bool get(int index) override;

    int length() override;
  };

  /**
   * Bits impl of the specified length with no bits set.
   */
  class MatchNoBits : public std::enable_shared_from_this<MatchNoBits>,
                      public Bits
  {
    GET_CLASS_NAME(MatchNoBits)
  public:
    const int len;

    MatchNoBits(int len);

    bool get(int index) override;

    int length() override;
  };
} // namespace org::apache::lucene::util
