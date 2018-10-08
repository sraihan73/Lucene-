#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Sort;
}

namespace org::apache::lucene::search
{
template <typename T>
class FieldComparator;
}
namespace org::apache::lucene::search
{
class DocComparator;
}
namespace org::apache::lucene::util::packed
{
class PackedLongValues;
}
namespace org::apache::lucene::search
{
class SortField;
}
namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
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
using TimSorter = org::apache::lucene::util::TimSorter;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * Sorts documents of a given index by returning a permutation on the document
 * IDs.
 * @lucene.experimental
 */
class Sorter final : public std::enable_shared_from_this<Sorter>
{
  GET_CLASS_NAME(Sorter)
public:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<Sort> sort_;

  /** Creates a new Sorter to sort the index with {@code sort} */
  Sorter(std::shared_ptr<Sort> sort);

  /**
   * A permutation of doc IDs. For every document ID between <tt>0</tt> and
   * {@link IndexReader#maxDoc()}, <code>oldToNew(newToOld(docID))</code> must
   * return <code>docID</code>.
   */
public:
  class DocMap : public std::enable_shared_from_this<DocMap>
  {
    GET_CLASS_NAME(DocMap)

    /** Given a doc ID from the original index, return its ordinal in the
     *  sorted index. */
  public:
    virtual int oldToNew(int docID) = 0;

    /** Given the ordinal of a doc ID, return its doc ID in the original index.
     */
    virtual int newToOld(int docID) = 0;

    /** Return the number of documents in this map_obj. This must be equal to the
     *  {@link org.apache.lucene.index.LeafReader#maxDoc() number of documents}
     * of the
     *  {@link org.apache.lucene.index.LeafReader} which is sorted. */
    virtual int size() = 0;
  };

  /** Check consistency of a {@link DocMap}, useful for assertions. */
public:
  static bool isConsistent(std::shared_ptr<DocMap> docMap);

  /** A comparator of doc IDs. */
public:
  class DocComparator : public std::enable_shared_from_this<DocComparator>
  {
    GET_CLASS_NAME(DocComparator)

    /** Compare docID1 against docID2. The contract for the return value is the
     *  same as {@link Comparator#compare(Object, Object)}. */
  public:
    virtual int compare(int docID1, int docID2) = 0;
  };

private:
  class DocValueSorter final : public TimSorter
  {
    GET_CLASS_NAME(DocValueSorter)

  private:
    std::deque<int> const docs;
    const std::shared_ptr<Sorter::DocComparator> comparator;
    std::deque<int> const tmp;

  public:
    DocValueSorter(std::deque<int> &docs,
                   std::shared_ptr<Sorter::DocComparator> comparator);

  protected:
    int compare(int i, int j) override;

    void swap(int i, int j) override;

    void copy(int src, int dest) override;

    void save(int i, int len) override;

    void restore(int i, int j) override;

    int compareSaved(int i, int j) override;

  protected:
    std::shared_ptr<DocValueSorter> shared_from_this()
    {
      return std::static_pointer_cast<DocValueSorter>(
          org.apache.lucene.util.TimSorter::shared_from_this());
    }
  };

  /** Computes the old-to-new permutation over the given comparator. */
private:
  static std::shared_ptr<Sorter::DocMap>
  sort(int const maxDoc,
       std::shared_ptr<FieldComparator::DocComparator> comparator);

private:
  class DocMapAnonymousInnerClass : public Sorter::DocMap
  {
    GET_CLASS_NAME(DocMapAnonymousInnerClass)
  private:
    int maxDoc = 0;
    std::shared_ptr<PackedLongValues> newToOld;
    std::shared_ptr<PackedLongValues> oldToNew;

  public:
    DocMapAnonymousInnerClass(int maxDoc,
                              std::shared_ptr<PackedLongValues> newToOld,
                              std::shared_ptr<PackedLongValues> oldToNew);

    int oldToNew(int docID) override;

    int newToOld(int docID) override;

    int size() override;

  protected:
    std::shared_ptr<DocMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocMapAnonymousInnerClass>(
          Sorter.DocMap::shared_from_this());
    }
  };

  /** Returns the native sort type for {@link SortedSetSortField} and {@link
   * SortedNumericSortField},
   * {@link SortField#getType()} otherwise */
public:
  static SortField::Type getSortFieldType(std::shared_ptr<SortField> sortField);

  /** Wraps a {@link SortedNumericDocValues} as a single-valued view if the
   * field is an instance of {@link SortedNumericSortField}, returns {@link
   * NumericDocValues} for the field otherwise. */
  static std::shared_ptr<NumericDocValues>
  getOrWrapNumeric(std::shared_ptr<LeafReader> reader,
                   std::shared_ptr<SortField> sortField) ;

  /** Wraps a {@link SortedSetDocValues} as a single-valued view if the field is
   * an instance of {@link SortedSetSortField},
   * returns {@link SortedDocValues} for the field otherwise. */
  static std::shared_ptr<SortedDocValues>
  getOrWrapSorted(std::shared_ptr<LeafReader> reader,
                  std::shared_ptr<SortField> sortField) ;

  static std::shared_ptr<FieldComparator::DocComparator>
  getDocComparator(std::shared_ptr<LeafReader> reader,
                   std::shared_ptr<SortField> sortField) ;

public:
  class NumericDocValuesSupplier
  {
    GET_CLASS_NAME(NumericDocValuesSupplier)
  public:
    virtual std::shared_ptr<NumericDocValues> get() = 0;
  };

public:
  class SortedDocValuesSupplier
  {
    GET_CLASS_NAME(SortedDocValuesSupplier)
  public:
    virtual std::shared_ptr<SortedDocValues> get() = 0;
  };

  /** We cannot use the {@link FieldComparator} API because that API requires
   * that you send it docIDs in order.  Note that this API allocates
   * arrays[maxDoc] to hold the native values needed for comparison, but 1) they
   * are transient (only alive while sorting this one segment), and 2) in the
   * typical index sorting case, they are only used to sort newly flushed
   * segments, which will be smaller than merged segments.  */
public:
  static std::shared_ptr<FieldComparator::DocComparator>
  getDocComparator(int maxDoc, std::shared_ptr<SortField> sortField,
                   std::shared_ptr<SortedDocValuesSupplier> sortedProvider,
                   std::shared_ptr<NumericDocValuesSupplier>
                       numericProvider) ;

private:
  class DocComparatorAnonymousInnerClass : public DocComparator
  {
    GET_CLASS_NAME(DocComparatorAnonymousInnerClass)
  private:
    int reverseMul = 0;
    std::deque<int> ords;

  public:
    DocComparatorAnonymousInnerClass(int reverseMul, std::deque<int> &ords);

    int compare(int docID1, int docID2) override;

  protected:
    std::shared_ptr<DocComparatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocComparatorAnonymousInnerClass>(
          DocComparator::shared_from_this());
    }
  };

private:
  class DocComparatorAnonymousInnerClass2 : public DocComparator
  {
    GET_CLASS_NAME(DocComparatorAnonymousInnerClass2)
  private:
    int reverseMul = 0;
    std::deque<int64_t> values;

  public:
    DocComparatorAnonymousInnerClass2(int reverseMul,
                                      std::deque<int64_t> &values);

    int compare(int docID1, int docID2) override;

  protected:
    std::shared_ptr<DocComparatorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<DocComparatorAnonymousInnerClass2>(
          DocComparator::shared_from_this());
    }
  };

private:
  class DocComparatorAnonymousInnerClass3 : public DocComparator
  {
    GET_CLASS_NAME(DocComparatorAnonymousInnerClass3)
  private:
    int reverseMul = 0;
    std::deque<int> values;

  public:
    DocComparatorAnonymousInnerClass3(int reverseMul, std::deque<int> &values);

    int compare(int docID1, int docID2) override;

  protected:
    std::shared_ptr<DocComparatorAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<DocComparatorAnonymousInnerClass3>(
          DocComparator::shared_from_this());
    }
  };

private:
  class DocComparatorAnonymousInnerClass4 : public DocComparator
  {
    GET_CLASS_NAME(DocComparatorAnonymousInnerClass4)
  private:
    int reverseMul = 0;
    std::deque<double> values;

  public:
    DocComparatorAnonymousInnerClass4(int reverseMul,
                                      std::deque<double> &values);

    int compare(int docID1, int docID2) override;

  protected:
    std::shared_ptr<DocComparatorAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<DocComparatorAnonymousInnerClass4>(
          DocComparator::shared_from_this());
    }
  };

private:
  class DocComparatorAnonymousInnerClass5 : public DocComparator
  {
    GET_CLASS_NAME(DocComparatorAnonymousInnerClass5)
  private:
    int reverseMul = 0;
    std::deque<float> values;

  public:
    DocComparatorAnonymousInnerClass5(int reverseMul,
                                      std::deque<float> &values);

    int compare(int docID1, int docID2) override;

  protected:
    std::shared_ptr<DocComparatorAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<DocComparatorAnonymousInnerClass5>(
          DocComparator::shared_from_this());
    }
  };

  /**
   * Returns a mapping from the old document ID to its new location in the
   * sorted index. Implementations can use the auxiliary
   * {@link #sort(int, DocComparator)} to compute the old-to-new permutation
   * given a deque of documents and their corresponding values.
   * <p>
   * A return value of <tt>null</tt> is allowed and means that
   * <code>reader</code> is already sorted.
   * <p>
   * <b>NOTE:</b> deleted documents are expected to appear in the mapping as
   * well, they will however be marked as deleted in the sorted view.
   */
public:
  std::shared_ptr<DocMap>
  sort(std::shared_ptr<LeafReader> reader) ;

  std::shared_ptr<DocMap>
  sort(int maxDoc, std::deque<std::shared_ptr<FieldComparator::DocComparator>>
                       &comparators) ;

private:
  class DocComparatorAnonymousInnerClass6 : public DocComparator
  {
    GET_CLASS_NAME(DocComparatorAnonymousInnerClass6)
  private:
    std::shared_ptr<Sorter> outerInstance;

    std::deque<std::shared_ptr<FieldComparator::DocComparator>> comparators;

  public:
    DocComparatorAnonymousInnerClass6(
        std::shared_ptr<Sorter> outerInstance,
        std::deque<std::shared_ptr<FieldComparator::DocComparator>>
            &comparators);

    int compare(int docID1, int docID2) override;

  protected:
    std::shared_ptr<DocComparatorAnonymousInnerClass6> shared_from_this()
    {
      return std::static_pointer_cast<DocComparatorAnonymousInnerClass6>(
          DocComparator::shared_from_this());
    }
  };

  /**
   * Returns the identifier of this {@link Sorter}.
   * <p>This identifier is similar to {@link Object#hashCode()} and should be
   * chosen so that two instances of this class that sort documents likewise
   * will have the same identifier. On the contrary, this identifier should be
   * different on different {@link Sort sorts}.
   */
public:
  std::wstring getID();

  virtual std::wstring toString();

  static const std::shared_ptr<Scorer> FAKESCORER;

private:
  class ScorerAnonymousInnerClass : public Scorer
  {
    GET_CLASS_NAME(ScorerAnonymousInnerClass)
  public:
    ScorerAnonymousInnerClass();

    float score = 0;
    int doc = -1;

    int docID() override;

    std::shared_ptr<DocIdSetIterator> iterator() override;

    float score()  override;

  protected:
    std::shared_ptr<ScorerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ScorerAnonymousInnerClass>(
          org.apache.lucene.search.Scorer::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::index
