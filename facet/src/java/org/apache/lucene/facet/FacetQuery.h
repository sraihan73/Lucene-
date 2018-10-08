#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"

#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/facet/DimConfig.h"

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

using Term = org::apache::lucene::index::Term;
using TermQuery = org::apache::lucene::search::TermQuery;

/**
 * A term {@link Query} over a {@link FacetField}.
 * <p>
 * <b>NOTE:</b>This helper class is an alternative to {@link DrillDownQuery}
 * especially in cases where you don't intend to use {@link DrillSideways}
GET_CLASS_NAME(is)
 *
 * @lucene.experimental
 */
class FacetQuery : public TermQuery
{
  GET_CLASS_NAME(FacetQuery)

  /**
   * Creates a new {@code FacetQuery} filtering the query on the given
   * dimension.
   */
public:
  FacetQuery(std::shared_ptr<FacetsConfig> facetsConfig,
             const std::wstring &dimension, std::deque<std::wstring> &path);

  /**
   * Creates a new {@code FacetQuery} filtering the query on the given
   * dimension. <p> <b>NOTE:</b>Uses FacetsConfig.DEFAULT_DIM_CONFIG.
   */
  FacetQuery(const std::wstring &dimension, std::deque<std::wstring> &path);

  static std::shared_ptr<Term>
  toTerm(std::shared_ptr<FacetsConfig::DimConfig> dimConfig,
         const std::wstring &dimension, std::deque<std::wstring> &path);

protected:
  std::shared_ptr<FacetQuery> shared_from_this()
  {
    return std::static_pointer_cast<FacetQuery>(
        org.apache.lucene.search.TermQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
