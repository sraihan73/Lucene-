#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/Facets.h"

#include  "core/src/java/org/apache/lucene/facet/FacetResult.h"

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

/** Maps specified dims to provided Facets impls; else, uses
 *  the default Facets impl. */
class MultiFacets : public Facets
{
  GET_CLASS_NAME(MultiFacets)
private:
  const std::unordered_map<std::wstring, std::shared_ptr<Facets>> dimToFacets;
  const std::shared_ptr<Facets> defaultFacets;

  /** Create this, with no default {@link Facets}. */
public:
  MultiFacets(
      std::unordered_map<std::wstring, std::shared_ptr<Facets>> &dimToFacets);

  /** Create this, with the specified default {@link Facets}
   *  for fields not included in {@code dimToFacets}. */
  MultiFacets(
      std::unordered_map<std::wstring, std::shared_ptr<Facets>> &dimToFacets,
      std::shared_ptr<Facets> defaultFacets);

  std::shared_ptr<FacetResult>
  getTopChildren(int topN, const std::wstring &dim,
                 std::deque<std::wstring> &path)  override;

  std::shared_ptr<Number>
  getSpecificValue(const std::wstring &dim,
                   std::deque<std::wstring> &path)  override;

  std::deque<std::shared_ptr<FacetResult>>
  getAllDims(int topN)  override;

protected:
  std::shared_ptr<MultiFacets> shared_from_this()
  {
    return std::static_pointer_cast<MultiFacets>(Facets::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
