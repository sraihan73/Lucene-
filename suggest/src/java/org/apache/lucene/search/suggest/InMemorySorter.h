#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRefArray;
}

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
namespace org::apache::lucene::search::suggest
{

using BytesRefSorter =
    org::apache::lucene::search::suggest::fst::BytesRefSorter;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefArray = org::apache::lucene::util::BytesRefArray;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using Counter = org::apache::lucene::util::Counter;

/**
 * An {@link BytesRefSorter} that keeps all the entries in memory.
 * @lucene.experimental
 * @lucene.internal
 */
class InMemorySorter final
    : public std::enable_shared_from_this<InMemorySorter>,
      public BytesRefSorter
{
  GET_CLASS_NAME(InMemorySorter)
private:
  const std::shared_ptr<BytesRefArray> buffer =
      std::make_shared<BytesRefArray>(Counter::newCounter());
  bool closed = false;
  const std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comparator;

  /**
   * Creates an InMemorySorter, sorting entries by the
   * provided comparator.
   */
public:
  InMemorySorter(
      std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comparator);

  void add(std::shared_ptr<BytesRef> utf8) override;

  std::shared_ptr<BytesRefIterator> iterator() override;

  std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
  getComparator() override;
};

} // namespace org::apache::lucene::search::suggest
