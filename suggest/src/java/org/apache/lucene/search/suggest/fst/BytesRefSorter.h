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
namespace org::apache::lucene::search::suggest::fst
{

using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;

/**
 * Collects {@link BytesRef} and then allows one to iterate over their sorted
 * order. Implementations of this interface will be called in a single-threaded
 * scenario.
 * @lucene.experimental
 * @lucene.internal
 */
class BytesRefSorter
{
  GET_CLASS_NAME(BytesRefSorter)
  /**
   * Adds a single suggestion entry (possibly compound with its bucket).
   *
   * @throws IOException If an I/O exception occurs.
   * @throws IllegalStateException If an addition attempt is performed after
   * a call to {@link #iterator()} has been made.
   */
public:
  virtual void add(std::shared_ptr<BytesRef> utf8) = 0;

  /**
   * Sorts the entries added in {@link #add(BytesRef)} and returns
   * an iterator over all sorted entries.
   *
   * @throws IOException If an I/O exception occurs.
   */
  virtual std::shared_ptr<BytesRefIterator> iterator() = 0;

  /**
   * Comparator used to determine the sort order of entries.
   */
  virtual std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
  getComparator() = 0;
};

} // namespace org::apache::lucene::search::suggest::fst
