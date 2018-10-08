#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class CodecReader;
}

namespace org::apache::lucene::index
{
class MergeState;
}
namespace org::apache::lucene::index
{
class DocMap;
}
namespace org::apache::lucene::search
{
class Sort;
}
namespace org::apache::lucene::index
{
class LeafAndDocID;
}
namespace org::apache::lucene::index
{
class ComparableProvider;
}
namespace org::apache::lucene::util::packed
{
class PackedLongValues;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::search
{
class SortField;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
}
namespace org::apache::lucene::util
{
class LongValues;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
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

namespace org::apache::lucene::index
{

using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using Bits = org::apache::lucene::util::Bits;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) final class
// MultiSorter
class MultiSorter final : public std::enable_shared_from_this<MultiSorter>
{

  /** Does a merge sort of the leaves of the incoming reader, returning {@link
   *DocMap} to map_obj each leaf's documents into the merged segment.  The documents
   *for each incoming leaf reader must already be sorted by the same sort!
   *  Returns null if the merge sort is not needed (segments are already in
   *index sort order).
   **/
public:
  static std::deque<std::shared_ptr<MergeState::DocMap>>
  sort(std::shared_ptr<Sort> sort,
       std::deque<std::shared_ptr<CodecReader>> &readers) ;

private:
  class PriorityQueueAnonymousInnerClass
      : public PriorityQueue<std::shared_ptr<LeafAndDocID>>
  {
    GET_CLASS_NAME(PriorityQueueAnonymousInnerClass)
  private:
    std::deque<std::deque<std::shared_ptr<
        org::apache::lucene::index::MultiSorter::ComparableProvider>>>
        comparables;

  public:
    PriorityQueueAnonymousInnerClass(
        int leafCount,
        std::deque<std::deque<std::shared_ptr<
            org::apache::lucene::index::MultiSorter::ComparableProvider>>>
            &comparables);

    bool lessThan(std::shared_ptr<LeafAndDocID> a,
                  std::shared_ptr<LeafAndDocID> b) override;

  protected:
    std::shared_ptr<PriorityQueueAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PriorityQueueAnonymousInnerClass>(
          org.apache.lucene.util
              .PriorityQueue<LeafAndDocID>::shared_from_this());
    }
  };

private:
  class DocMapAnonymousInnerClass : public MergeState::DocMap
  {
    GET_CLASS_NAME(DocMapAnonymousInnerClass)
  private:
    std::shared_ptr<PackedLongValues> remapped;
    std::shared_ptr<Bits> liveDocs;

  public:
    DocMapAnonymousInnerClass(std::shared_ptr<PackedLongValues> remapped,
                              std::shared_ptr<Bits> liveDocs);

    int get(int docID) override;

  protected:
    std::shared_ptr<DocMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocMapAnonymousInnerClass>(
          MergeState.DocMap::shared_from_this());
    }
  };

private:
  class LeafAndDocID : public std::enable_shared_from_this<LeafAndDocID>
  {
    GET_CLASS_NAME(LeafAndDocID)
  public:
    const int readerIndex;
    const std::shared_ptr<Bits> liveDocs;
    const int maxDoc;
    std::deque<Comparable> const values;
    int docID = 0;

    LeafAndDocID(int readerIndex, std::shared_ptr<Bits> liveDocs, int maxDoc,
                 int numComparables);
  };

  /** Returns an object for this docID whose .compareTo represents the requested
   * {@link SortField} sort order. */
private:
  class ComparableProvider
  {
    GET_CLASS_NAME(ComparableProvider)
  public:
    virtual Comparable getComparable(int docID) = 0;
  };

  /** Returns {@code ComparableProvider}s for the provided readers to represent
   * the requested {@link SortField} sort order. */
private:
  static std::deque<std::shared_ptr<ComparableProvider>>
  getComparableProviders(
      std::deque<std::shared_ptr<CodecReader>> &readers,
      std::shared_ptr<SortField> sortField) ;

private:
  class ComparableProviderAnonymousInnerClass
      : public std::enable_shared_from_this<
            ComparableProviderAnonymousInnerClass>,
        public ComparableProvider
  {
    GET_CLASS_NAME(ComparableProviderAnonymousInnerClass)
  private:
    int reverseMul = 0;
    int missingOrd = 0;
    std::shared_ptr<org::apache::lucene::index::SortedDocValues> readerValues;
    std::shared_ptr<LongValues> globalOrds;

  public:
    ComparableProviderAnonymousInnerClass(
        int reverseMul, int missingOrd,
        std::shared_ptr<org::apache::lucene::index::SortedDocValues>
            readerValues,
        std::shared_ptr<LongValues> globalOrds);

    // used only by assert:
    int lastDocID = -1;

  private:
    bool docsInOrder(int docID);

  public:
    Comparable getComparable(int docID)  override;
  };

private:
  class ComparableProviderAnonymousInnerClass2
      : public std::enable_shared_from_this<
            ComparableProviderAnonymousInnerClass2>,
        public ComparableProvider
  {
    GET_CLASS_NAME(ComparableProviderAnonymousInnerClass2)
  private:
    int reverseMul = 0;
    std::optional<int64_t> missingValue;
    std::shared_ptr<org::apache::lucene::index::NumericDocValues> values;

  public:
    ComparableProviderAnonymousInnerClass2(
        int reverseMul, std::optional<int64_t> &missingValue,
        std::shared_ptr<org::apache::lucene::index::NumericDocValues> values);

    // used only by assert:
    int lastDocID = -1;

  private:
    bool docsInOrder(int docID);

  public:
    Comparable getComparable(int docID)  override;
  };

private:
  class ComparableProviderAnonymousInnerClass3
      : public std::enable_shared_from_this<
            ComparableProviderAnonymousInnerClass3>,
        public ComparableProvider
  {
    GET_CLASS_NAME(ComparableProviderAnonymousInnerClass3)
  private:
    int reverseMul = 0;
    std::optional<int> missingValue;
    std::shared_ptr<org::apache::lucene::index::NumericDocValues> values;

  public:
    ComparableProviderAnonymousInnerClass3(
        int reverseMul, std::optional<int> &missingValue,
        std::shared_ptr<org::apache::lucene::index::NumericDocValues> values);

    // used only by assert:
    int lastDocID = -1;

  private:
    bool docsInOrder(int docID);

  public:
    Comparable getComparable(int docID)  override;
  };

private:
  class ComparableProviderAnonymousInnerClass4
      : public std::enable_shared_from_this<
            ComparableProviderAnonymousInnerClass4>,
        public ComparableProvider
  {
    GET_CLASS_NAME(ComparableProviderAnonymousInnerClass4)
  private:
    int reverseMul = 0;
    std::optional<double> missingValue;
    std::shared_ptr<org::apache::lucene::index::NumericDocValues> values;

  public:
    ComparableProviderAnonymousInnerClass4(
        int reverseMul, std::optional<double> &missingValue,
        std::shared_ptr<org::apache::lucene::index::NumericDocValues> values);

    // used only by assert:
    int lastDocID = -1;

  private:
    bool docsInOrder(int docID);

  public:
    Comparable getComparable(int docID)  override;
  };

private:
  class ComparableProviderAnonymousInnerClass5
      : public std::enable_shared_from_this<
            ComparableProviderAnonymousInnerClass5>,
        public ComparableProvider
  {
    GET_CLASS_NAME(ComparableProviderAnonymousInnerClass5)
  private:
    int reverseMul = 0;
    std::optional<float> missingValue;
    std::shared_ptr<org::apache::lucene::index::NumericDocValues> values;

  public:
    ComparableProviderAnonymousInnerClass5(
        int reverseMul, std::optional<float> &missingValue,
        std::shared_ptr<org::apache::lucene::index::NumericDocValues> values);

    // used only by assert:
    int lastDocID = -1;

  private:
    bool docsInOrder(int docID);

  public:
    Comparable getComparable(int docID)  override;
  };
};

} // namespace org::apache::lucene::index
