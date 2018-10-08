#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
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

/** Common base class for all facets implementations.
 *
 *  @lucene.experimental */
class Facets : public std::enable_shared_from_this<Facets>
{
  GET_CLASS_NAME(Facets)

  /** Default constructor. */
public:
  Facets();

  /** Returns the topN child labels under the specified
   *  path.  Returns null if the specified path doesn't
   *  exist or if this dimension was never seen. */
  virtual std::shared_ptr<FacetResult>
  getTopChildren(int topN, const std::wstring &dim,
                 std::deque<std::wstring> &path) = 0;

  /** Return the count or value
   *  for a specific path.  Returns -1 if
   *  this path doesn't exist, else the count. */
  virtual std::shared_ptr<Number>
  getSpecificValue(const std::wstring &dim,
                   std::deque<std::wstring> &path) = 0;

  /** Returns topN labels for any dimension that had hits,
   *  sorted by the number of hits that dimension matched;
   *  this is used for "sparse" faceting, where many
   *  different dimensions were indexed, for example
   *  depending on the type of document. */
  virtual std::deque<std::shared_ptr<FacetResult>> getAllDims(int topN) = 0;
};

} // #include  "core/src/java/org/apache/lucene/facet/
