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
 * This class implements a simple char deque with access to the underlying
 * array.
 *
 * This class has been taken from GPLv3 FOP project
 * (http://xmlgraphics.apache.org/fop/). They have been slightly modified.
 */
class CharVector : public std::enable_shared_from_this<CharVector>,
                   public Cloneable
{
  GET_CLASS_NAME(CharVector)

  /**
   * Capacity increment size
   */
private:
  static constexpr int DEFAULT_BLOCK_SIZE = 2048;

  int blockSize = 0;

  /**
   * The encapsulated array
   */
  std::deque<wchar_t> array_;

  /**
   * Points to next free item
   */
  int n = 0;

public:
  CharVector();

  CharVector(int capacity);

  CharVector(std::deque<wchar_t> &a);

  CharVector(std::deque<wchar_t> &a, int capacity);

  /**
   * Reset Vector but don't resize or clear elements
   */
  virtual void clear();

  std::shared_ptr<CharVector> clone() override;

  virtual std::deque<wchar_t> getArray();

  /**
   * return number of items in array
   */
  virtual int length();

  /**
   * returns current capacity of array
   */
  virtual int capacity();

  virtual void put(int index, wchar_t val);

  virtual wchar_t get(int index);

  virtual int alloc(int size);

  virtual void trimToSize();
};

} // namespace org::apache::lucene::analysis::compound::hyphenation
