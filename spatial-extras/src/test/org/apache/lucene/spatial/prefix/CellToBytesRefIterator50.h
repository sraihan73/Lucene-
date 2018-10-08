#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/Cell.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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

/** For testing Lucene <= 5.0. Index redundant prefixes for leaf cells. Fixed in
 * LUCENE-4942. */
class CellToBytesRefIterator50 : public CellToBytesRefIterator
{
  GET_CLASS_NAME(CellToBytesRefIterator50)

public:
  std::shared_ptr<Cell> repeatCell;

  std::shared_ptr<BytesRef> next() override;

protected:
  std::shared_ptr<CellToBytesRefIterator50> shared_from_this()
  {
    return std::static_pointer_cast<CellToBytesRefIterator50>(
        CellToBytesRefIterator::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/
