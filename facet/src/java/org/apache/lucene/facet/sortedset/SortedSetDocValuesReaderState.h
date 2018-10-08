#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SortedSetDocValues;
}

namespace org::apache::lucene::index
{
class IndexReader;
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
namespace org::apache::lucene::facet::sortedset
{

using IndexReader = org::apache::lucene::index::IndexReader;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Accountable = org::apache::lucene::util::Accountable;

/** Wraps a {@link IndexReader} and resolves ords
 *  using existing {@link SortedSetDocValues} APIs without a
 *  separate taxonomy index.  This only supports flat facets
 *  (dimension + label), and it makes faceting a bit
 *  slower, adds some cost at reopen time, but avoids
 *  managing the separate taxonomy index.  It also requires
 *  less RAM than the taxonomy index, as it manages the flat
 *  (2-level) hierarchy more efficiently.  In addition, the
 *  tie-break during faceting is now meaningful (in label
 *  sorted order).
 *
 *  <p><b>NOTE</b>: creating an instance of this class is
 *  somewhat costly, as it computes per-segment ordinal maps,
 *  so you should create it once and re-use that one instance
 *  for a given {@link IndexReader}. */

class SortedSetDocValuesReaderState
    : public std::enable_shared_from_this<SortedSetDocValuesReaderState>,
      public Accountable
{
  GET_CLASS_NAME(SortedSetDocValuesReaderState)

  /** Holds start/end range of ords, which maps to one
   *  dimension (someday we may generalize it to map_obj to
   *  hierarchies within one dimension). */
public:
  class OrdRange final : public std::enable_shared_from_this<OrdRange>
  {
    GET_CLASS_NAME(OrdRange)
    /** Start of range, inclusive: */
  public:
    const int start;
    /** End of range, inclusive: */
    const int end;

    /** Start and end are inclusive. */
    OrdRange(int start, int end);
  };

  /** Sole constructor. */
protected:
  SortedSetDocValuesReaderState();

  /** Return top-level doc values. */
public:
  virtual std::shared_ptr<SortedSetDocValues> getDocValues() = 0;

  /** Indexed field we are reading. */
  virtual std::wstring getField() = 0;

  /** Returns the {@link OrdRange} for this dimension. */
  virtual std::shared_ptr<OrdRange> getOrdRange(const std::wstring &dim) = 0;

  /** Returns mapping from prefix to {@link OrdRange}. */
  virtual std::unordered_map<std::wstring, std::shared_ptr<OrdRange>>
  getPrefixToOrdRange() = 0;

  /** Returns top-level index reader. */
  virtual std::shared_ptr<IndexReader> getReader() = 0;

  /** Number of unique labels. */
  virtual int getSize() = 0;
};

} // namespace org::apache::lucene::facet::sortedset
