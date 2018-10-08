#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::grouping
{
class GroupedFacetHit;
}

namespace org::apache::lucene::util
{
class SentinelIntSet;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search::grouping
{
class SegmentResult;
}
namespace org::apache::lucene::index
{
class TermsEnum;
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
namespace org::apache::lucene::search::grouping
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using SentinelIntSet = org::apache::lucene::util::SentinelIntSet;

/**
 * An implementation of {@link GroupFacetCollector} that computes grouped facets
 * based on the indexed terms from DocValues.
 *
 * @lucene.experimental
 */
class TermGroupFacetCollector : public GroupFacetCollector
{
  GET_CLASS_NAME(TermGroupFacetCollector)

public:
  const std::deque<std::shared_ptr<GroupedFacetHit>> groupedFacetHits;
  const std::shared_ptr<SentinelIntSet> segmentGroupedFacetHits;

  std::shared_ptr<SortedDocValues> groupFieldTermsIndex;

  /**
   * Factory method for creating the right implementation based on the fact
   * whether the facet field contains multiple tokens per documents.
   *
   * @param groupField The group field
   * @param facetField The facet field
   * @param facetFieldMultivalued Whether the facet field has multiple tokens
   * per document
   * @param facetPrefix The facet prefix a facet entry should start with to be
   * included.
   * @param initialSize The initial allocation size of the internal int set and
   * group facet deque which should roughly match the total number of expected
   * unique groups. Be aware that the heap usage is 4 bytes * initialSize.
   * @return <code>TermGroupFacetCollector</code> implementation
   */
  static std::shared_ptr<TermGroupFacetCollector> createTermGroupFacetCollector(
      const std::wstring &groupField, const std::wstring &facetField,
      bool facetFieldMultivalued, std::shared_ptr<BytesRef> facetPrefix,
      int initialSize);

  TermGroupFacetCollector(const std::wstring &groupField,
                          const std::wstring &facetField,
                          std::shared_ptr<BytesRef> facetPrefix,
                          int initialSize);

  // Implementation for single valued facet fields.
public:
  class SV;

  // Implementation for multi valued facet fields.
public:
  class MV;

private:
  class GroupedFacetHit : public std::enable_shared_from_this<GroupedFacetHit>
  {
    GET_CLASS_NAME(GroupedFacetHit)

  public:
    const std::shared_ptr<BytesRef> groupValue;
    const std::shared_ptr<BytesRef> facetValue;

    GroupedFacetHit(std::shared_ptr<BytesRef> groupValue,
                    std::shared_ptr<BytesRef> facetValue);
  };

protected:
  std::shared_ptr<TermGroupFacetCollector> shared_from_this()
  {
    return std::static_pointer_cast<TermGroupFacetCollector>(
        GroupFacetCollector::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::grouping
class TermGroupFacetCollector::SV : public TermGroupFacetCollector
{
  GET_CLASS_NAME(TermGroupFacetCollector::SV)

private:
  std::shared_ptr<SortedDocValues> facetFieldTermsIndex;

public:
  SV(const std::wstring &groupField, const std::wstring &facetField,
     std::shared_ptr<BytesRef> facetPrefix, int initialSize);

  void collect(int doc)  override;

protected:
  void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
      IOException) override;

  std::shared_ptr<SegmentResult>
  createSegmentResult()  override;

private:
  class SegmentResult : public GroupFacetCollector::SegmentResult
  {
    GET_CLASS_NAME(SegmentResult)

  public:
    const std::shared_ptr<TermsEnum> tenum;

    SegmentResult(std::deque<int> &counts, int total,
                  std::shared_ptr<TermsEnum> tenum, int startFacetOrd,
                  int endFacetOrd) ;

  protected:
    void nextTerm()  override;

  protected:
    std::shared_ptr<SegmentResult> shared_from_this()
    {
      return std::static_pointer_cast<SegmentResult>(
          GroupFacetCollector.SegmentResult::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SV> shared_from_this()
  {
    return std::static_pointer_cast<SV>(
        TermGroupFacetCollector::shared_from_this());
  }
};
class TermGroupFacetCollector::MV : public TermGroupFacetCollector
{
  GET_CLASS_NAME(TermGroupFacetCollector::MV)

private:
  std::shared_ptr<SortedSetDocValues> facetFieldDocTermOrds;
  std::shared_ptr<TermsEnum> facetOrdTermsEnum;
  int facetFieldNumTerms = 0;

public:
  MV(const std::wstring &groupField, const std::wstring &facetField,
     std::shared_ptr<BytesRef> facetPrefix, int initialSize);

  void collect(int doc)  override;

private:
  void process(int groupOrd, int facetOrd) ;

protected:
  void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
      IOException) override;

  std::shared_ptr<SegmentResult>
  createSegmentResult()  override;

private:
  class SegmentResult : public GroupFacetCollector::SegmentResult
  {
    GET_CLASS_NAME(SegmentResult)

  public:
    const std::shared_ptr<TermsEnum> tenum;

    SegmentResult(std::deque<int> &counts, int total, int missingCountIndex,
                  std::shared_ptr<TermsEnum> tenum, int startFacetOrd,
                  int endFacetOrd) ;

  protected:
    void nextTerm()  override;

  protected:
    std::shared_ptr<SegmentResult> shared_from_this()
    {
      return std::static_pointer_cast<SegmentResult>(
          GroupFacetCollector.SegmentResult::shared_from_this());
    }
  };

protected:
  std::shared_ptr<MV> shared_from_this()
  {
    return std::static_pointer_cast<MV>(
        TermGroupFacetCollector::shared_from_this());
  }
};
