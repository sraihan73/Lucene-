#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"

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
namespace org::apache::lucene::facet
{

using TermInSetQuery = org::apache::lucene::search::TermInSetQuery;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A multi-terms {@link Query} over a {@link FacetField}.
 * <p>
 * <b>NOTE:</b>This helper class is an alternative to {@link DrillDownQuery}
 * especially in cases where you don't intend to use {@link DrillSideways}
GET_CLASS_NAME(is)
 *
 * @lucene.experimental
 * @see org.apache.lucene.search.TermInSetQuery
 */
class MultiFacetQuery : public TermInSetQuery
{
  GET_CLASS_NAME(MultiFacetQuery)

  /**
   * Creates a new {@code MultiFacetQuery} filtering the query on the given
   * dimension.
   */
public:
  MultiFacetQuery(std::shared_ptr<FacetsConfig> facetsConfig,
                  const std::wstring &dimension,
                  std::deque<std::wstring> &paths);

  /**
   * Creates a new {@code MultiFacetQuery} filtering the query on the given
   * dimension. <p> <b>NOTE:</b>Uses FacetsConfig.DEFAULT_DIM_CONFIG.
   */
  MultiFacetQuery(const std::wstring &dimension,
                  std::deque<std::wstring> &paths);

  static std::shared_ptr<std::deque<std::shared_ptr<BytesRef>>>
  toTerms(const std::wstring &dimension, std::deque<std::wstring> &paths);

protected:
  std::shared_ptr<MultiFacetQuery> shared_from_this()
  {
    return std::static_pointer_cast<MultiFacetQuery>(
        org.apache.lucene.search.TermInSetQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
