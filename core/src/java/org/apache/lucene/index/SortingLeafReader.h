#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Sorter.h"

#include  "core/src/java/org/apache/lucene/index/DocMap.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/automaton/CompiledAutomaton.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/index/CachedBinaryDVs.h"
#include  "core/src/java/org/apache/lucene/index/CachedNumericDVs.h"
#include  "core/src/java/org/apache/lucene/util/BitSet.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/index/IntersectVisitor.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/store/RAMFile.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/search/Sort.h"
#include  "core/src/java/org/apache/lucene/index/StoredFieldVisitor.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"

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
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMFile = org::apache::lucene::store::RAMFile;
using BitSet = org::apache::lucene::util::BitSet;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using TimSorter = org::apache::lucene::util::TimSorter;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.SortedSetDocValues.NO_MORE_ORDS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * An {@link org.apache.lucene.index.LeafReader} which supports sorting
 * documents by a given
 * {@link Sort}. This is package private and is only used by Lucene for BWC when
 * it needs to merge an unsorted flushed segment built by an older version
 * (newly flushed segments are sorted since version 7.0).
 *
 * @lucene.experimental
 */
class SortingLeafReader : public FilterLeafReader
{
  GET_CLASS_NAME(SortingLeafReader)

  // TODO remove from here; move to FreqProxTermsWriter or FreqProxFields?
public:
  class SortingFields : public FilterFields
  {
    GET_CLASS_NAME(SortingFields)

  private:
    const std::shared_ptr<Sorter::DocMap> docMap;
    const std::shared_ptr<FieldInfos> infos;

  public:
    SortingFields(std::shared_ptr<Fields> in_,
                  std::shared_ptr<FieldInfos> infos,
                  std::shared_ptr<Sorter::DocMap> docMap);

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

  protected:
    std::shared_ptr<SortingFields> shared_from_this()
    {
      return std::static_pointer_cast<SortingFields>(
          FilterFields::shared_from_this());
    }
  };

private:
  class SortingTerms : public FilterTerms
  {
    GET_CLASS_NAME(SortingTerms)

  private:
    const std::shared_ptr<Sorter::DocMap> docMap;
    const IndexOptions indexOptions;

  public:
    SortingTerms(std::shared_ptr<Terms> in_, IndexOptions indexOptions,
                 std::shared_ptr<Sorter::DocMap> docMap);

    std::shared_ptr<TermsEnum> iterator()  override;

    std::shared_ptr<TermsEnum>
    intersect(std::shared_ptr<CompiledAutomaton> compiled,
              std::shared_ptr<BytesRef> startTerm)  override;

  protected:
    std::shared_ptr<SortingTerms> shared_from_this()
    {
      return std::static_pointer_cast<SortingTerms>(
          FilterTerms::shared_from_this());
    }
  };

private:
  class SortingTermsEnum : public FilterTermsEnum
  {
    GET_CLASS_NAME(SortingTermsEnum)

  public:
    const std::shared_ptr<Sorter::DocMap>
        docMap; // pkg-protected to avoid synthetic accessor methods
  private:
    const IndexOptions indexOptions;
    const bool hasPositions;

  public:
    SortingTermsEnum(std::shared_ptr<TermsEnum> in_,
                     std::shared_ptr<Sorter::DocMap> docMap,
                     IndexOptions indexOptions, bool hasPositions);

    std::shared_ptr<PostingsEnum>
    postings(std::shared_ptr<PostingsEnum> reuse,
             int const flags)  override;

  protected:
    std::shared_ptr<SortingTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<SortingTermsEnum>(
          FilterTermsEnum::shared_from_this());
    }
  };

public:
  class SortingBinaryDocValues : public BinaryDocValues
  {
    GET_CLASS_NAME(SortingBinaryDocValues)

  private:
    const std::shared_ptr<CachedBinaryDVs> dvs;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;

  public:
    SortingBinaryDocValues(std::shared_ptr<CachedBinaryDVs> dvs);

    int nextDoc() override;

    int docID() override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    std::shared_ptr<BytesRef> binaryValue() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SortingBinaryDocValues> shared_from_this()
    {
      return std::static_pointer_cast<SortingBinaryDocValues>(
          BinaryDocValues::shared_from_this());
    }
  };

private:
  const std::unordered_map<std::wstring, std::shared_ptr<CachedNumericDVs>>
      cachedNumericDVs =
          std::unordered_map<std::wstring, std::shared_ptr<CachedNumericDVs>>();

public:
  class CachedNumericDVs : public std::enable_shared_from_this<CachedNumericDVs>
  {
    GET_CLASS_NAME(CachedNumericDVs)
  private:
    std::deque<int64_t> const values;
    const std::shared_ptr<BitSet> docsWithField;

  public:
    CachedNumericDVs(std::deque<int64_t> &values,
                     std::shared_ptr<BitSet> docsWithField);
  };

private:
  const std::unordered_map<std::wstring, std::shared_ptr<CachedBinaryDVs>>
      cachedBinaryDVs =
          std::unordered_map<std::wstring, std::shared_ptr<CachedBinaryDVs>>();

public:
  class CachedBinaryDVs : public std::enable_shared_from_this<CachedBinaryDVs>
  {
    GET_CLASS_NAME(CachedBinaryDVs)
    // TODO: at least cutover to BytesRefArray here:
  private:
    std::deque<std::shared_ptr<BytesRef>> const values;
    const std::shared_ptr<BitSet> docsWithField;

  public:
    CachedBinaryDVs(std::deque<std::shared_ptr<BytesRef>> &values,
                    std::shared_ptr<BitSet> docsWithField);
  };

private:
  const std::unordered_map<std::wstring, std::deque<int>> cachedSortedDVs =
      std::unordered_map<std::wstring, std::deque<int>>();

public:
  class SortingNumericDocValues : public NumericDocValues
  {
    GET_CLASS_NAME(SortingNumericDocValues)

  private:
    const std::shared_ptr<CachedNumericDVs> dvs;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;

  public:
    SortingNumericDocValues(std::shared_ptr<CachedNumericDVs> dvs);

    int docID() override;

    int nextDoc() override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t longValue() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SortingNumericDocValues> shared_from_this()
    {
      return std::static_pointer_cast<SortingNumericDocValues>(
          NumericDocValues::shared_from_this());
    }
  };

private:
  class SortingBits : public std::enable_shared_from_this<SortingBits>,
                      public Bits
  {
    GET_CLASS_NAME(SortingBits)

  private:
    const std::shared_ptr<Bits> in_;
    const std::shared_ptr<Sorter::DocMap> docMap;

  public:
    SortingBits(std::shared_ptr<Bits> in_,
                std::shared_ptr<Sorter::DocMap> docMap);

    bool get(int index) override;

    int length() override;
  };

private:
  class SortingPointValues : public PointValues
  {
    GET_CLASS_NAME(SortingPointValues)

  private:
    const std::shared_ptr<PointValues> in_;
    const std::shared_ptr<Sorter::DocMap> docMap;

  public:
    SortingPointValues(std::shared_ptr<PointValues> in_,
                       std::shared_ptr<Sorter::DocMap> docMap);

    void intersect(std::shared_ptr<IntersectVisitor> visitor) throw(
        IOException) override;

  private:
    class IntersectVisitorAnonymousInnerClass
        : public std::enable_shared_from_this<
              IntersectVisitorAnonymousInnerClass>,
          public IntersectVisitor
    {
      GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
    private:
      std::shared_ptr<SortingPointValues> outerInstance;

      std::shared_ptr<org::apache::lucene::index::PointValues::IntersectVisitor>
          visitor;

    public:
      IntersectVisitorAnonymousInnerClass(
          std::shared_ptr<SortingPointValues> outerInstance,
          std::shared_ptr<
              org::apache::lucene::index::PointValues::IntersectVisitor>
              visitor);

      void visit(int docID)  override;

      void visit(int docID,
                 std::deque<char> &packedValue)  override;

      Relation compare(std::deque<char> &minPackedValue,
                       std::deque<char> &maxPackedValue) override;
    };

  public:
    int64_t
    estimatePointCount(std::shared_ptr<IntersectVisitor> visitor) override;

    std::deque<char> getMinPackedValue()  override;

    std::deque<char> getMaxPackedValue()  override;

    int getNumDimensions()  override;

    int getBytesPerDimension()  override;

    int64_t size() override;

    int getDocCount() override;

  protected:
    std::shared_ptr<SortingPointValues> shared_from_this()
    {
      return std::static_pointer_cast<SortingPointValues>(
          PointValues::shared_from_this());
    }
  };

public:
  class SortingSortedDocValues : public SortedDocValues
  {
    GET_CLASS_NAME(SortingSortedDocValues)

  private:
    const std::shared_ptr<SortedDocValues> in_;
    std::deque<int> const ords;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;

  public:
    SortingSortedDocValues(std::shared_ptr<SortedDocValues> in_,
                           std::deque<int> &ords);

    int docID() override;

    int nextDoc() override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int ordValue() override;

    int64_t cost() override;

    std::shared_ptr<BytesRef> lookupOrd(int ord)  override;

    int getValueCount() override;

  protected:
    std::shared_ptr<SortingSortedDocValues> shared_from_this()
    {
      return std::static_pointer_cast<SortingSortedDocValues>(
          SortedDocValues::shared_from_this());
    }
  };

  // TODO: pack long[][] into an int[] (offset) and long[] instead:
private:
  const std::unordered_map<std::wstring, std::deque<std::deque<int64_t>>>
      cachedSortedSetDVs =
          std::unordered_map<std::wstring,
                             std::deque<std::deque<int64_t>>>();

public:
  class SortingSortedSetDocValues : public SortedSetDocValues
  {
    GET_CLASS_NAME(SortingSortedSetDocValues)

  private:
    const std::shared_ptr<SortedSetDocValues> in_;
    std::deque<std::deque<int64_t>> const ords;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;
    int ordUpto = 0;

  public:
    SortingSortedSetDocValues(std::shared_ptr<SortedSetDocValues> in_,
                              std::deque<std::deque<int64_t>> &ords);

    int docID() override;

    int nextDoc() override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t nextOrd() override;

    int64_t cost() override;

    std::shared_ptr<BytesRef>
    lookupOrd(int64_t ord)  override;

    int64_t getValueCount() override;

  protected:
    std::shared_ptr<SortingSortedSetDocValues> shared_from_this()
    {
      return std::static_pointer_cast<SortingSortedSetDocValues>(
          SortedSetDocValues::shared_from_this());
    }
  };

private:
  const std::unordered_map<std::wstring, std::deque<std::deque<int64_t>>>
      cachedSortedNumericDVs =
          std::unordered_map<std::wstring,
                             std::deque<std::deque<int64_t>>>();

public:
  class SortingSortedNumericDocValues : public SortedNumericDocValues
  {
    GET_CLASS_NAME(SortingSortedNumericDocValues)
  private:
    const std::shared_ptr<SortedNumericDocValues> in_;
    std::deque<std::deque<int64_t>> const values;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;
    int upto = 0;

  public:
    SortingSortedNumericDocValues(std::shared_ptr<SortedNumericDocValues> in_,
                                  std::deque<std::deque<int64_t>> &values);

    int docID() override;

    int nextDoc() override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t nextValue() override;

    int64_t cost() override;

    int docValueCount() override;

  protected:
    std::shared_ptr<SortingSortedNumericDocValues> shared_from_this()
    {
      return std::static_pointer_cast<SortingSortedNumericDocValues>(
          SortedNumericDocValues::shared_from_this());
    }
  };

public:
  class SortingDocsEnum : public FilterPostingsEnum
  {
    GET_CLASS_NAME(SortingDocsEnum)

  private:
    class DocFreqSorter final : public TimSorter
    {
      GET_CLASS_NAME(DocFreqSorter)

    private:
      std::deque<int> docs;
      std::deque<int> freqs;
      std::deque<int> const tmpDocs;
      std::deque<int> tmpFreqs;

    public:
      DocFreqSorter(int maxDoc);

      void reset(std::deque<int> &docs, std::deque<int> &freqs) override;

    protected:
      int compare(int i, int j) override;

      void swap(int i, int j) override;

      void copy(int src, int dest) override;

      void save(int i, int len) override;

      void restore(int i, int j) override;

      int compareSaved(int i, int j) override;

    protected:
      std::shared_ptr<DocFreqSorter> shared_from_this()
      {
        return std::static_pointer_cast<DocFreqSorter>(
            org.apache.lucene.util.TimSorter::shared_from_this());
      }
    };

  private:
    const int maxDoc;
    const std::shared_ptr<DocFreqSorter> sorter;
    std::deque<int> docs;
    std::deque<int> freqs;
    int docIt = -1;
    const int upto;
    const bool withFreqs;

  public:
    SortingDocsEnum(int maxDoc, std::shared_ptr<SortingDocsEnum> reuse,
                    std::shared_ptr<PostingsEnum> in_, bool withFreqs,
                    std::shared_ptr<Sorter::DocMap> docMap) ;

    // for testing
    virtual bool reused(std::shared_ptr<PostingsEnum> other);

    int advance(int const target)  override;

    int docID() override;

    int freq()  override;

    int nextDoc()  override;

    /** Returns the wrapped {@link PostingsEnum}. */
    virtual std::shared_ptr<PostingsEnum> getWrapped();

    // we buffer up docs/freqs only, don't forward any positions requests to
    // underlying enum

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

  protected:
    std::shared_ptr<SortingDocsEnum> shared_from_this()
    {
      return std::static_pointer_cast<SortingDocsEnum>(
          FilterPostingsEnum::shared_from_this());
    }
  };

public:
  class SortingPostingsEnum : public FilterPostingsEnum
  {
    GET_CLASS_NAME(SortingPostingsEnum)

    /**
     * A {@link TimSorter} which sorts two parallel arrays of doc IDs and
     * offsets in one go. Everytime a doc ID is 'swapped', its corresponding
     * offset is swapped too.
     */
  private:
    class DocOffsetSorter final : public TimSorter
    {
      GET_CLASS_NAME(DocOffsetSorter)

    private:
      std::deque<int> docs;
      std::deque<int64_t> offsets;
      std::deque<int> const tmpDocs;
      std::deque<int64_t> const tmpOffsets;

    public:
      DocOffsetSorter(int maxDoc);

      void reset(std::deque<int> &docs, std::deque<int64_t> &offsets);

    protected:
      int compare(int i, int j) override;

      void swap(int i, int j) override;

      void copy(int src, int dest) override;

      void save(int i, int len) override;

      void restore(int i, int j) override;

      int compareSaved(int i, int j) override;

    protected:
      std::shared_ptr<DocOffsetSorter> shared_from_this()
      {
        return std::static_pointer_cast<DocOffsetSorter>(
            org.apache.lucene.util.TimSorter::shared_from_this());
      }
    };

  private:
    const int maxDoc;
    const std::shared_ptr<DocOffsetSorter> sorter;
    std::deque<int> docs;
    std::deque<int64_t> offsets;
    const int upto;

    const std::shared_ptr<IndexInput> postingInput;
    const bool storeOffsets;

    int docIt = -1;
    int pos = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int startOffset_ = -1;
    // C++ NOTE: Fields cannot have the same name as methods:
    int endOffset_ = -1;
    const std::shared_ptr<BytesRef> payload;
    int currFreq = 0;

    const std::shared_ptr<RAMFile> file;

  public:
    SortingPostingsEnum(int maxDoc, std::shared_ptr<SortingPostingsEnum> reuse,
                        std::shared_ptr<PostingsEnum> in_,
                        std::shared_ptr<Sorter::DocMap> docMap,
                        bool storeOffsets) ;

    // for testing
    virtual bool reused(std::shared_ptr<PostingsEnum> other);

  private:
    void addPositions(std::shared_ptr<PostingsEnum> in_,
                      std::shared_ptr<IndexOutput> out) ;

  public:
    int advance(int const target)  override;

    int docID() override;

    int endOffset()  override;

    int freq()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

    int nextDoc()  override;

    int nextPosition()  override;

    int startOffset()  override;

    /** Returns the wrapped {@link PostingsEnum}. */
    virtual std::shared_ptr<PostingsEnum> getWrapped();

  protected:
    std::shared_ptr<SortingPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<SortingPostingsEnum>(
          FilterPostingsEnum::shared_from_this());
    }
  };

  /** Return a sorted view of <code>reader</code> according to the order
   *  defined by <code>sort</code>. If the reader is already sorted, this
   *  method might return the reader as-is. */
public:
  static std::shared_ptr<LeafReader>
  wrap(std::shared_ptr<LeafReader> reader,
       std::shared_ptr<Sort> sort) ;

  /** Expert: same as {@link #wrap(org.apache.lucene.index.LeafReader, Sort)}
   * but operates directly on a {@link Sorter.DocMap}. */
  static std::shared_ptr<LeafReader>
  wrap(std::shared_ptr<LeafReader> reader,
       std::shared_ptr<Sorter::DocMap> docMap);

  const std::shared_ptr<Sorter::DocMap>
      docMap; // pkg-protected to avoid synthetic accessor methods

private:
  SortingLeafReader(std::shared_ptr<LeafReader> in_,
                    std::shared_ptr<Sorter::DocMap> docMap);

public:
  void document(
      int const docID,
      std::shared_ptr<StoredFieldVisitor> visitor)  override;

  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  std::shared_ptr<BinaryDocValues>
  getBinaryDocValues(const std::wstring &field)  override;

  std::shared_ptr<Bits> getLiveDocs() override;

  std::shared_ptr<PointValues>
  getPointValues(const std::wstring &fieldName)  override;

private:
  const std::unordered_map<std::wstring, std::shared_ptr<CachedNumericDVs>>
      cachedNorms =
          std::unordered_map<std::wstring, std::shared_ptr<CachedNumericDVs>>();

public:
  std::shared_ptr<NumericDocValues>
  getNormValues(const std::wstring &field)  override;

  std::shared_ptr<NumericDocValues>
  getNumericDocValues(const std::wstring &field)  override;

  std::shared_ptr<SortedNumericDocValues> getSortedNumericDocValues(
      const std::wstring &field)  override;

  std::shared_ptr<SortedDocValues>
  getSortedDocValues(const std::wstring &field)  override;

  std::shared_ptr<SortedSetDocValues>
  getSortedSetDocValues(const std::wstring &field)  override;

  std::shared_ptr<Fields>
  getTermVectors(int const docID)  override;

  virtual std::wstring toString();

  // no caching on sorted views

  std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

protected:
  std::shared_ptr<SortingLeafReader> shared_from_this()
  {
    return std::static_pointer_cast<SortingLeafReader>(
        FilterLeafReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
