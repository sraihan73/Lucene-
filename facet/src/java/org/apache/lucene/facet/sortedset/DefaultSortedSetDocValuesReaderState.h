#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::index
{
class OrdinalMap;
}
namespace org::apache::lucene::facet::sortedset
{
class SortedSetDocValuesReaderState;
}
namespace org::apache::lucene::facet::sortedset
{
class OrdRange;
}
namespace org::apache::lucene::util
{
class Accountable;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
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

using OrdRange = org::apache::lucene::facet::sortedset::
    SortedSetDocValuesReaderState::OrdRange;
using IndexReader = org::apache::lucene::index::IndexReader;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Accountable = org::apache::lucene::util::Accountable;

/**
 * Default implementation of {@link SortedSetDocValuesFacetCounts}. You must
 * ensure the original
 * {@link IndexReader} passed to the constructor is not closed whenever you use
 * this class!
 */
class DefaultSortedSetDocValuesReaderState
    : public SortedSetDocValuesReaderState
{
  GET_CLASS_NAME(DefaultSortedSetDocValuesReaderState)

private:
  const std::wstring field;
  const int valueCount;

  /** {@link IndexReader} passed to the constructor. */
public:
  const std::shared_ptr<IndexReader> reader;

private:
  const std::unordered_map<std::wstring, std::shared_ptr<OrdinalMap>>
      cachedOrdMaps =
          std::unordered_map<std::wstring, std::shared_ptr<OrdinalMap>>();

  const std::unordered_map<std::wstring, std::shared_ptr<OrdRange>>
      prefixToOrdRange =
          std::unordered_map<std::wstring, std::shared_ptr<OrdRange>>();

  /** Creates this, pulling doc values from the default {@link
   *  FacetsConfig#DEFAULT_INDEX_FIELD_NAME}. */
public:
  DefaultSortedSetDocValuesReaderState(
      std::shared_ptr<IndexReader> reader) ;

  /** Creates this, pulling doc values from the specified
   *  field. */
  DefaultSortedSetDocValuesReaderState(
      std::shared_ptr<IndexReader> reader,
      const std::wstring &field) ;

  /**
   * Return the memory usage of this object in bytes. Negative values are
   * illegal.
   */
  int64_t ramBytesUsed() override;

  /**
   * Returns nested resources of this class.
   * The result should be a point-in-time snapshot (to avoid race conditions).
   * @see Accountables
   */
  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  virtual std::wstring toString();

  /** Return top-level doc values. */
  std::shared_ptr<SortedSetDocValues>
  getDocValues()  override;

  /** Returns mapping from prefix to {@link OrdRange}. */
  std::unordered_map<std::wstring, std::shared_ptr<OrdRange>>
  getPrefixToOrdRange() override;

  /** Returns the {@link OrdRange} for this dimension. */
  std::shared_ptr<OrdRange> getOrdRange(const std::wstring &dim) override;

  /** Indexed field we are reading. */
  std::wstring getField() override;

  std::shared_ptr<IndexReader> getReader() override;

  /** Number of unique labels. */
  int getSize() override;

protected:
  std::shared_ptr<DefaultSortedSetDocValuesReaderState> shared_from_this()
  {
    return std::static_pointer_cast<DefaultSortedSetDocValuesReaderState>(
        SortedSetDocValuesReaderState::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::sortedset
