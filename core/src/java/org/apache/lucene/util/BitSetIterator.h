#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}

namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::util
{
class SparseFixedBitSet;
}
namespace org::apache::lucene::util
{
class BitSet;
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

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

/**
 * A {@link DocIdSetIterator} which iterates over set bits in a
 * bit set.
 * @lucene.internal
 */
class BitSetIterator : public DocIdSetIterator
{
  GET_CLASS_NAME(BitSetIterator)

private:
  template <typename T>
  static T getBitSet(std::shared_ptr<DocIdSetIterator> iterator,
                     std::type_info clazz);

  /** If the provided iterator wraps a {@link FixedBitSet}, returns it,
   * otherwise returns null. */
public:
  static std::shared_ptr<FixedBitSet>
  getFixedBitSetOrNull(std::shared_ptr<DocIdSetIterator> iterator);

  /** If the provided iterator wraps a {@link SparseFixedBitSet}, returns it,
   * otherwise returns null. */
  static std::shared_ptr<SparseFixedBitSet>
  getSparseFixedBitSetOrNull(std::shared_ptr<DocIdSetIterator> iterator);

private:
  const std::shared_ptr<BitSet> bits;
  const int length;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t cost_;
  int doc = -1;

  /** Sole constructor. */
public:
  BitSetIterator(std::shared_ptr<BitSet> bits, int64_t cost);

  /** Return the wrapped {@link BitSet}. */
  virtual std::shared_ptr<BitSet> getBitSet();

  int docID() override;

  /** Set the current doc id that this iterator is on. */
  virtual void setDocId(int docId);

  int nextDoc() override;

  int advance(int target) override;

  int64_t cost() override;

protected:
  std::shared_ptr<BitSetIterator> shared_from_this()
  {
    return std::static_pointer_cast<BitSetIterator>(
        org.apache.lucene.search.DocIdSetIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
