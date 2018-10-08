#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

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
namespace org::apache::lucene::facet::taxonomy
{

/**
 * Returns 3 arrays for traversing the taxonomy:
 * <ul>
 * <li>{@code parents}: {@code parents[i]} denotes the parent of category
 * ordinal {@code i}.</li>
 * <li>{@code children}: {@code children[i]} denotes a child of category ordinal
 * {@code i}.</li>
 * <li>{@code siblings}: {@code siblings[i]} denotes the sibling of category
 * ordinal {@code i}.</li>
 * </ul>
 *
 * To traverse the taxonomy tree, you typically start with {@code children[0]}
 * (ordinal 0 is reserved for ROOT), and then depends if you want to do DFS or
 * BFS, you call {@code children[children[0]]} or {@code siblings[children[0]]}
 * and so forth, respectively.
 *
 * <p>
 * <b>NOTE:</b> you are not expected to modify the values of the arrays, since
 * the arrays are shared with other threads.
 *
 * @lucene.experimental
 */
class ParallelTaxonomyArrays
    : public std::enable_shared_from_this<ParallelTaxonomyArrays>
{
  GET_CLASS_NAME(ParallelTaxonomyArrays)

  /** Sole constructor. */
public:
  ParallelTaxonomyArrays();

  /**
   * Returns the parents array, where {@code parents[i]} denotes the parent of
   * category ordinal {@code i}.
   */
  virtual std::deque<int> parents() = 0;

  /**
   * Returns the children array, where {@code children[i]} denotes a child of
   * category ordinal {@code i}.
   */
  virtual std::deque<int> children() = 0;

  /**
   * Returns the siblings array, where {@code siblings[i]} denotes the sibling
   * of category ordinal {@code i}.
   */
  virtual std::deque<int> siblings() = 0;
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
