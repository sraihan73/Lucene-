#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

#include  "core/src/java/org/apache/lucene/util/Accountable.h"

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
namespace org::apache::lucene::facet::taxonomy::directory
{

using ParallelTaxonomyArrays =
    org::apache::lucene::facet::taxonomy::ParallelTaxonomyArrays;
using IndexReader = org::apache::lucene::index::IndexReader;
using Accountable = org::apache::lucene::util::Accountable;

/**
 * A {@link ParallelTaxonomyArrays} that are initialized from the taxonomy
 * index.
 *
 * @lucene.experimental
 */
class TaxonomyIndexArrays : public ParallelTaxonomyArrays, public Accountable
{
  GET_CLASS_NAME(TaxonomyIndexArrays)

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  std::deque<int> const parents_;

  // the following two arrays are lazily initialized. note that we only keep a
  // single bool member as volatile, instead of declaring the arrays
  // volatile. the code guarantees that only after the bool is set to true,
  // the arrays are returned.
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool initializedChildren = false;
  bool initializedChildren = false;
  // C++ NOTE: Fields cannot have the same name as methods:
  std::deque<int> children_, siblings_;

  /** Used by {@link #add(int, int)} after the array grew. */
  TaxonomyIndexArrays(std::deque<int> &parents);

public:
  TaxonomyIndexArrays(std::shared_ptr<IndexReader> reader) ;

  TaxonomyIndexArrays(
      std::shared_ptr<IndexReader> reader,
      std::shared_ptr<TaxonomyIndexArrays> copyFrom) ;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void initChildrenSiblings(std::shared_ptr<TaxonomyIndexArrays> copyFrom);

  void computeChildrenSiblings(int first);

  // Read the parents of the new categories
  void initParents(std::shared_ptr<IndexReader> reader,
                   int first) ;

  /**
   * Adds the given ordinal/parent info and returns either a new instance if the
   * underlying array had to grow, or this instance otherwise.
   * <p>
   * <b>NOTE:</b> you should call this method from a thread-safe code.
   */
public:
  virtual std::shared_ptr<TaxonomyIndexArrays> add(int ordinal,
                                                   int parentOrdinal);

  /**
   * Returns the parents array, where {@code parents[i]} denotes the parent of
   * category ordinal {@code i}.
   */
  std::deque<int> parents() override;

  /**
   * Returns the children array, where {@code children[i]} denotes the youngest
   * child of category ordinal {@code i}. The youngest child is defined as the
   * category that was added last to the taxonomy as an immediate child of
   * {@code i}.
   */
  std::deque<int> children() override;

  /**
   * Returns the siblings array, where {@code siblings[i]} denotes the sibling
   * of category ordinal {@code i}. The sibling is defined as the previous
   * youngest child of {@code parents[i]}.
   */
  std::deque<int> siblings() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t ramBytesUsed() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

protected:
  std::shared_ptr<TaxonomyIndexArrays> shared_from_this()
  {
    return std::static_pointer_cast<TaxonomyIndexArrays>(
        org.apache.lucene.facet.taxonomy
            .ParallelTaxonomyArrays::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/directory/
