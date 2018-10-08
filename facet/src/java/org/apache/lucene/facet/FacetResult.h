#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet
{
class LabelAndValue;
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
namespace org::apache::lucene::facet
{

/** Counts or aggregates for a single dimension. */
class FacetResult final : public std::enable_shared_from_this<FacetResult>
{
  GET_CLASS_NAME(FacetResult)

  /** Dimension that was requested. */
public:
  const std::wstring dim;

  /** Path whose children were requested. */
  std::deque<std::wstring> const path;

  /** Total value for this path (sum of all child counts, or
   *  sum of all child values), even those not included in
   *  the topN. */
  const std::shared_ptr<Number> value;

  /** How many child labels were encountered. */
  const int childCount;

  /** Child counts. */
  std::deque<std::shared_ptr<LabelAndValue>> const labelValues;

  /** Sole constructor. */
  FacetResult(const std::wstring &dim, std::deque<std::wstring> &path,
              std::shared_ptr<Number> value,
              std::deque<std::shared_ptr<LabelAndValue>> &labelValues,
              int childCount);

  virtual std::wstring toString();

  bool equals(std::any _other) override;

  virtual int hashCode();
};

} // namespace org::apache::lucene::facet
