#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/Bits.h"

#include  "core/src/java/org/apache/lucene/util/BitSet.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/util/BitSetIterator.h"

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
namespace org::apache::lucene::search::join
{

using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using BitSet = org::apache::lucene::util::BitSet;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using Bits = org::apache::lucene::util::Bits;

/** Select a value from a block of documents.
 *  @lucene.internal */
class BlockJoinSelector : public std::enable_shared_from_this<BlockJoinSelector>
{
  GET_CLASS_NAME(BlockJoinSelector)

private:
  BlockJoinSelector();

  /** Type of selection to perform. If none of the documents in the block have
   *  a value then no value will be selected. */
public:
  enum class Type {
    GET_CLASS_NAME(Type)
    /** Only consider the minimum value from the block when sorting. */
    MIN,
    /** Only consider the maximum value from the block when sorting. */
    MAX
  };

  /** Return a {@link Bits} instance that returns true if, and only if, any of
   *  the children of the given parent document has a value. */
public:
  static std::shared_ptr<Bits> wrap(std::shared_ptr<Bits> docsWithValue,
                                    std::shared_ptr<BitSet> parents,
                                    std::shared_ptr<BitSet> children);

private:
  class BitsAnonymousInnerClass
      : public std::enable_shared_from_this<BitsAnonymousInnerClass>,
        public Bits
  {
    GET_CLASS_NAME(BitsAnonymousInnerClass)
  private:
    std::shared_ptr<Bits> docsWithValue;
    std::shared_ptr<BitSet> parents;
    std::shared_ptr<BitSet> children;

  public:
    BitsAnonymousInnerClass(std::shared_ptr<Bits> docsWithValue,
                            std::shared_ptr<BitSet> parents,
                            std::shared_ptr<BitSet> children);

    bool get(int docID) override;

    int length() override;
  };

  /** Wraps the provided {@link SortedSetDocValues} in order to only select
   *  one value per parent among its {@code children} using the configured
   *  {@code selection} type. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // org.apache.lucene.index.SortedDocValues
  // wrap(org.apache.lucene.index.SortedSetDocValues sortedSet, Type selection,
  // org.apache.lucene.util.BitSet parents, org.apache.lucene.util.BitSet
  // children)
  static std::shared_ptr<SortedDocValues>
  wrap(std::shared_ptr<SortedSetDocValues> sortedSet, Type selection,
       std::shared_ptr<BitSet> parents, std::shared_ptr<BitSet> children);

  /** Wraps the provided {@link SortedSetDocValues} in order to only select
   *  one value per parent among its {@code children} using the configured
   *  {@code selection} type. */
  static std::shared_ptr<SortedDocValues>
  wrap(std::shared_ptr<SortedSetDocValues> sortedSet, Type selection,
       std::shared_ptr<BitSet> parents,
       std::shared_ptr<DocIdSetIterator> children);

  /** Wraps the provided {@link SortedDocValues} in order to only select
   *  one value per parent among its {@code children} using the configured
   *  {@code selection} type. */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // org.apache.lucene.index.SortedDocValues wrap(final
  // org.apache.lucene.index.SortedDocValues values, Type selection,
  // org.apache.lucene.util.BitSet parents, org.apache.lucene.util.BitSet
  // children)
  static std::shared_ptr<SortedDocValues>
  wrap(std::shared_ptr<SortedDocValues> values, Type selection,
       std::shared_ptr<BitSet> parents, std::shared_ptr<BitSet> children);

  /** Wraps the provided {@link SortedDocValues} in order to only select
   *  one value per parent among its {@code children} using the configured
   *  {@code selection} type. */
  static std::shared_ptr<SortedDocValues>
  wrap(std::shared_ptr<SortedDocValues> values, Type selection,
       std::shared_ptr<BitSet> parents,
       std::shared_ptr<DocIdSetIterator> children);

  /** Wraps the provided {@link SortedNumericDocValues} in order to only select
   *  one value per parent among its {@code children} using the configured
   *  {@code selection} type. */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // org.apache.lucene.index.NumericDocValues
  // wrap(org.apache.lucene.index.SortedNumericDocValues sortedNumerics, Type
  // selection, org.apache.lucene.util.BitSet parents,
  // org.apache.lucene.util.BitSet children)
  static std::shared_ptr<NumericDocValues>
  wrap(std::shared_ptr<SortedNumericDocValues> sortedNumerics, Type selection,
       std::shared_ptr<BitSet> parents, std::shared_ptr<BitSet> children);

  /** creates an iterator for the given bitset */
protected:
  static std::shared_ptr<BitSetIterator>
  toIter(std::shared_ptr<BitSet> children);

  /** Wraps the provided {@link SortedNumericDocValues} in order to only select
   *  one value per parent among its {@code children} using the configured
   *  {@code selection} type. */
public:
  static std::shared_ptr<NumericDocValues>
  wrap(std::shared_ptr<SortedNumericDocValues> sortedNumerics, Type selection,
       std::shared_ptr<BitSet> parents,
       std::shared_ptr<DocIdSetIterator> children);

  /** Wraps the provided {@link NumericDocValues}, iterating over only
   *  child documents, in order to only select one value per parent among
   *  its {@code children} using the configured {@code selection} type. */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static
  // org.apache.lucene.index.NumericDocValues wrap(final
  // org.apache.lucene.index.NumericDocValues values, Type selection,
  // org.apache.lucene.util.BitSet parents, org.apache.lucene.util.BitSet
  // children)
  static std::shared_ptr<NumericDocValues>
  wrap(std::shared_ptr<NumericDocValues> values, Type selection,
       std::shared_ptr<BitSet> parents, std::shared_ptr<BitSet> children);

  /** Wraps the provided {@link NumericDocValues}, iterating over only
   *  child documents, in order to only select one value per parent among
   *  its {@code children} using the configured {@code selection} type. */
  static std::shared_ptr<NumericDocValues>
  wrap(std::shared_ptr<NumericDocValues> values, Type selection,
       std::shared_ptr<BitSet> parents,
       std::shared_ptr<DocIdSetIterator> children);
};

} // #include  "core/src/java/org/apache/lucene/search/join/
