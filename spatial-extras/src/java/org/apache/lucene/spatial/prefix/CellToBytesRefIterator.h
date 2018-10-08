#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix::tree
{
class Cell;
}

namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::spatial::prefix
{

using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;

/**
 * A reset'able {@link org.apache.lucene.util.BytesRefIterator} wrapper around
 * an {@link java.util.Iterator} of {@link
 * org.apache.lucene.spatial.prefix.tree.Cell}s.
 *
 * @see PrefixTreeStrategy#newCellToBytesRefIterator()
 *
 * @lucene.internal
 */
class CellToBytesRefIterator
    : public std::enable_shared_from_this<CellToBytesRefIterator>,
      public BytesRefIterator
{
  GET_CLASS_NAME(CellToBytesRefIterator)

protected:
  std::shared_ptr<Iterator<std::shared_ptr<Cell>>> cellIter;
  std::shared_ptr<BytesRef> bytesRef = std::make_shared<BytesRef>();

public:
  virtual void reset(std::shared_ptr<Iterator<std::shared_ptr<Cell>>> cellIter);

  std::shared_ptr<BytesRef> next() override;
};

} // namespace org::apache::lucene::spatial::prefix
