#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DocIdSetIterator;
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
namespace org::apache::lucene::search
{

using Accountable = org::apache::lucene::util::Accountable;
using Bits = org::apache::lucene::util::Bits;

/**
 * A DocIdSet contains a set of doc ids. Implementing classes must
 * only implement {@link #iterator} to provide access to the set.
GET_CLASS_NAME(es)
 */
class DocIdSet : public std::enable_shared_from_this<DocIdSet>,
                 public Accountable
{
  GET_CLASS_NAME(DocIdSet)

  /** An empty {@code DocIdSet} instance */
public:
  static const std::shared_ptr<DocIdSet> EMPTY;

private:
  class DocIdSetAnonymousInnerClass;

  /** Provides a {@link DocIdSetIterator} to access the set.
   * This implementation can return <code>null</code> if there
   * are no docs that match. */
public:
  virtual std::shared_ptr<DocIdSetIterator> iterator() = 0;

  // TODO: somehow this class should express the cost of
  // iteration vs the cost of random access Bits; for
  // expensive Filters (e.g. distance < 1 km) we should use
  // bits() after all other Query/Filters have matched, but
  // this is the opposite of what bits() is for now
  // (down-low filtering using e.g. FixedBitSet)

  /** Optionally provides a {@link Bits} interface for random access
   * to matching documents.
   * @return {@code null}, if this {@code DocIdSet} does not support random
   * access. In contrast to {@link #iterator()}, a return value of {@code null}
   * <b>does not</b> imply that no documents match the filter!
   * The default implementation does not provide random access, so you
   * only need to implement this method if your DocIdSet can
   * guarantee random access to every docid in O(1) time without
   * external disk access (as {@link Bits} interface cannot throw
   * {@link IOException}). This is generally true for bit sets
   * like {@link org.apache.lucene.util.FixedBitSet}, which return
   * itself if they are used as {@code DocIdSet}.
   */
  virtual std::shared_ptr<Bits> bits() ;
};

} // namespace org::apache::lucene::search
class DocIdSet::DocIdSetAnonymousInnerClass : public DocIdSet
{
  GET_CLASS_NAME(DocIdSet::DocIdSetAnonymousInnerClass)
public:
  DocIdSetAnonymousInnerClass();

  std::shared_ptr<DocIdSetIterator> iterator() override;

  // we explicitly provide no random access, as this filter is 100% sparse and
  // iterator exits faster
  std::shared_ptr<Bits> bits() override;

  int64_t ramBytesUsed() override;

protected:
  std::shared_ptr<DocIdSetAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<DocIdSetAnonymousInnerClass>(
        DocIdSet::shared_from_this());
  }
};
