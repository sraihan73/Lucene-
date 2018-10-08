#pragma once
#include "stringhelper.h"
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

namespace org::apache::lucene::analysis::compound::hyphenation
{

/**
 * This class implements a simple byte deque with access to the underlying
 * array.
 * This class has been taken from GPLv3 FOP project
 * (http://xmlgraphics.apache.org/fop/). They have been slightly modified.
 */
class ByteVector : public std::enable_shared_from_this<ByteVector>
{
  GET_CLASS_NAME(ByteVector)

  /**
   * Capacity increment size
   */
private:
  static constexpr int DEFAULT_BLOCK_SIZE = 2048;

  int blockSize = 0;

  /**
   * The encapsulated array
   */
  std::deque<char> array_;

  /**
   * Points to next free item
   */
  int n = 0;

public:
  ByteVector();

  ByteVector(int capacity);

  ByteVector(std::deque<char> &a);

  ByteVector(std::deque<char> &a, int capacity);

  virtual std::deque<char> getArray();

  /**
   * return number of items in array
   */
  virtual int length();

  /**
   * returns current capacity of array
   */
  virtual int capacity();

  virtual void put(int index, char val);

  virtual char get(int index);

  /**
   * This is to implement memory allocation in the array. Like malloc().
   */
  virtual int alloc(int size);

  virtual void trimToSize();
};

} // namespace org::apache::lucene::analysis::compound::hyphenation
