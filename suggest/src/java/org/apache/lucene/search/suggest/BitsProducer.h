#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class LeafReaderContext;
}

namespace org::apache::lucene::util
{
class Bits;
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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Bits = org::apache::lucene::util::Bits;

/** A producer of {@link Bits} per segment. */
class BitsProducer : public std::enable_shared_from_this<BitsProducer>
{
  GET_CLASS_NAME(BitsProducer)

  /** Sole constructor, typically invoked by sub-classes. */
protected:
  BitsProducer();

  /** Return {@link Bits} for the given leaf. The returned instance must
   *  be non-null and have a {@link Bits#length() length} equal to
   *  {@link LeafReader#maxDoc() maxDoc}. */
public:
  virtual std::shared_ptr<Bits>
  getBits(std::shared_ptr<LeafReaderContext> context) = 0;
};

} // namespace org::apache::lucene::search::suggest
