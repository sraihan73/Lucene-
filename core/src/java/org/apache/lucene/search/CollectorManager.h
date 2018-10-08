#pragma once
#include "stringhelper.h"
#include <memory>
#include <type_traits>

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

/**
 * A manager of collectors. This class is useful to parallelize execution of
 * search requests and has two main methods:
 * <ul>
 *   <li>{@link #newCollector()} which must return a NEW collector which
 *       will be used to collect a certain set of leaves.</li>
 *   <li>{@link #reduce(std::deque)} which will be used to reduce the
 *       results of individual collections into a meaningful result.
 *       This method is only called after all leaves have been fully
 *       collected.</li>
 * </ul>
 *
 * @see IndexSearcher#search(Query, CollectorManager)
 * @lucene.experimental
 */
template <typename C, typename T>
class CollectorManager
{
  GET_CLASS_NAME(CollectorManager)
  static_assert(std::is_base_of<Collector, C>::value,
                L"C must inherit from Collector");

  /**
   * Return a new {@link Collector}. This must return a different instance on
   * each call.
   */
public:
  virtual C newCollector() = 0;

  /**
   * Reduce the results of individual collectors into a meaningful result.
   * For instance a {@link TopDocsCollector} would compute the
   * {@link TopDocsCollector#topDocs() top docs} of each collector and then
   * merge them using {@link TopDocs#merge(int, TopDocs[])}.
   * This method must be called after collection is finished on all provided
   * collectors.
   */
  virtual T reduce(std::shared_ptr<std::deque<C>> collectors) = 0;
};

} // #include  "core/src/java/org/apache/lucene/search/
