#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::util
{
class BytesRefIterator;
}

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

class SortableBytesRefArray
{
  GET_CLASS_NAME(SortableBytesRefArray)
  /** Append a new value */
public:
  virtual int append(std::shared_ptr<BytesRef> bytes) = 0;
  /** Clear all previously stored values */
  virtual void clear() = 0;
  /** Returns the number of values appended so far */
  virtual int size() = 0;
  /** Sort all values by the provided comparator and return an iterator over the
   * sorted values */
  virtual std::shared_ptr<BytesRefIterator>
  iterator(std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comp) = 0;
};

} // namespace org::apache::lucene::util
