#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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
 * Base implementation for a bit set.
 * @lucene.internal
 */
class BitSet : public std::enable_shared_from_this<BitSet>,
               public Bits,
               public Accountable
{
  GET_CLASS_NAME(BitSet)

  /** Build a {@link BitSet} from the content of the provided {@link
   * DocIdSetIterator}. NOTE: this will fully consume the {@link
   * DocIdSetIterator}. */
public:
  static std::shared_ptr<BitSet> of(std::shared_ptr<DocIdSetIterator> it,
                                    int maxDoc) ;

  /** Set the bit at <code>i</code>. */
  virtual void set(int i) = 0;

  /** Clear the bit at <code>i</code>. */
  virtual void clear(int i) = 0;

  /** Clears a range of bits.
   *
   * @param startIndex lower index
   * @param endIndex one-past the last bit to clear
   */
  virtual void clear(int startIndex, int endIndex) = 0;

  /**
   * Return the number of bits that are set.
   * NOTE: this method is likely to run in linear time
   */
  virtual int cardinality() = 0;

  /**
   * Return an approximation of the cardinality of this set. Some
   * implementations may trade accuracy for speed if they have the ability to
   * estimate the cardinality of the set without iterating over all the data.
   * The default implementation returns {@link #cardinality()}.
   */
  virtual int approximateCardinality();

  /** Returns the index of the last set bit before or on the index specified.
   *  -1 is returned if there are no more set bits.
   */
  virtual int prevSetBit(int index) = 0;

  /** Returns the index of the first set bit starting at the index specified.
   *  {@link DocIdSetIterator#NO_MORE_DOCS} is returned if there are no more set
   * bits.
   */
  virtual int nextSetBit(int index) = 0;

  /** Assert that the current doc is -1. */
protected:
  void checkUnpositioned(std::shared_ptr<DocIdSetIterator> iter);

  /** Does in-place OR of the bits provided by the iterator. The state of the
   *  iterator after this operation terminates is undefined. */
public:
  virtual void or (std::shared_ptr<DocIdSetIterator> iter) ;
};

} // #include  "core/src/java/org/apache/lucene/util/
