#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"

#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/index/StoredFieldVisitor.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/index/LeafMetaData.h"

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

using AttributeSource = org::apache::lucene::util::AttributeSource;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;

/**  A <code>FilterLeafReader</code> contains another LeafReader, which it
 * uses as its basic source of data, possibly transforming the data along the
 * way or providing additional functionality. The class
 * <code>FilterLeafReader</code> itself simply implements all abstract methods
 * of <code>IndexReader</code> with versions that pass all requests to the
 * contained index reader. Subclasses of <code>FilterLeafReader</code> may
 * further override some of these methods and may also provide additional
 * methods and fields.
 * <p><b>NOTE</b>: If you override {@link #getLiveDocs()}, you will likely need
 * to override {@link #numDocs()} as well and vice-versa.
 * <p><b>NOTE</b>: If this {@link FilterLeafReader} does not change the
 * content the contained reader, you could consider delegating calls to
 * {@link #getCoreCacheHelper()} and {@link #getReaderCacheHelper()}.
 */
class FilterLeafReader : public LeafReader
{
  GET_CLASS_NAME(FilterLeafReader)

  /** Get the wrapped instance by <code>reader</code> as long as this reader is
   *  an instance of {@link FilterLeafReader}.  */
public:
  static std::shared_ptr<LeafReader> unwrap(std::shared_ptr<LeafReader> reader);

  /** Base class for filtering {@link Fields}
   *  implementations. */
public:
  class FilterFields : public Fields
  {
    GET_CLASS_NAME(FilterFields)
    /** The underlying Fields instance. */
  protected:
    const std::shared_ptr<Fields> in_;

    /**
     * Creates a new FilterFields.
     * @param in the underlying Fields instance.
     */
  public:
    FilterFields(std::shared_ptr<Fields> in_);

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

    int size() override;

  protected:
    std::shared_ptr<FilterFields> shared_from_this()
    {
      return std::static_pointer_cast<FilterFields>(Fields::shared_from_this());
    }
  };

  /** Base class for filtering {@link Terms} implementations.
   * <p><b>NOTE</b>: If the order of terms and documents is not changed, and if
   * these terms are going to be intersected with automata, you could consider
   * overriding {@link #intersect} for better performance.
   */
public:
  class FilterTerms : public Terms
  {
    GET_CLASS_NAME(FilterTerms)
    /** The underlying Terms instance. */
  protected:
    const std::shared_ptr<Terms> in_;

    /**
     * Creates a new FilterTerms
     * @param in the underlying Terms instance.
     */
  public:
    FilterTerms(std::shared_ptr<Terms> in_);

    std::shared_ptr<TermsEnum> iterator()  override;

    int64_t size()  override;

    int64_t getSumTotalTermFreq()  override;

    int64_t getSumDocFreq()  override;

    int getDocCount()  override;

    bool hasFreqs() override;

    bool hasOffsets() override;

    bool hasPositions() override;

    bool hasPayloads() override;

    std::any getStats()  override;

  protected:
    std::shared_ptr<FilterTerms> shared_from_this()
    {
      return std::static_pointer_cast<FilterTerms>(Terms::shared_from_this());
    }
  };

  /** Base class for filtering {@link TermsEnum} implementations. */
public:
  class FilterTermsEnum : public TermsEnum
  {
    GET_CLASS_NAME(FilterTermsEnum)
    /** The underlying TermsEnum instance. */
  protected:
    const std::shared_ptr<TermsEnum> in_;

    /**
     * Creates a new FilterTermsEnum
     * @param in the underlying TermsEnum instance.
     */
  public:
    FilterTermsEnum(std::shared_ptr<TermsEnum> in_);

    std::shared_ptr<AttributeSource> attributes() override;

    SeekStatus
    seekCeil(std::shared_ptr<BytesRef> text)  override;

    void seekExact(int64_t ord)  override;

    std::shared_ptr<BytesRef> next()  override;

    std::shared_ptr<BytesRef> term()  override;

    int64_t ord()  override;

    int docFreq()  override;

    int64_t totalTermFreq()  override;

    std::shared_ptr<PostingsEnum>
    postings(std::shared_ptr<PostingsEnum> reuse,
             int flags)  override;

  protected:
    std::shared_ptr<FilterTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<FilterTermsEnum>(
          TermsEnum::shared_from_this());
    }
  };

  /** Base class for filtering {@link PostingsEnum} implementations. */
public:
  class FilterPostingsEnum : public PostingsEnum
  {
    GET_CLASS_NAME(FilterPostingsEnum)
    /** The underlying PostingsEnum instance. */
  protected:
    const std::shared_ptr<PostingsEnum> in_;

    /**
     * Create a new FilterPostingsEnum
     * @param in the underlying PostingsEnum instance.
     */
  public:
    FilterPostingsEnum(std::shared_ptr<PostingsEnum> in_);

    std::shared_ptr<AttributeSource> attributes() override;

    int docID() override;

    int freq()  override;

    int nextDoc()  override;

    int advance(int target)  override;

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<FilterPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<FilterPostingsEnum>(
          PostingsEnum::shared_from_this());
    }
  };

  /** The underlying LeafReader. */
protected:
  const std::shared_ptr<LeafReader> in_;

  /**
   * <p>Construct a FilterLeafReader based on the specified base reader.
   * <p>Note that base reader is closed if this FilterLeafReader is closed.</p>
   * @param in specified base reader.
   */
public:
  FilterLeafReader(std::shared_ptr<LeafReader> in_);

  std::shared_ptr<Bits> getLiveDocs() override;

  std::shared_ptr<FieldInfos> getFieldInfos() override;

  std::shared_ptr<PointValues>
  getPointValues(const std::wstring &field)  override;

  std::shared_ptr<Fields> getTermVectors(int docID)  override;

  int numDocs() override;

  int maxDoc() override;

  void document(int docID, std::shared_ptr<StoredFieldVisitor> visitor) throw(
      IOException) override;

protected:
  void doClose()  override;

public:
  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  virtual std::wstring toString();

  std::shared_ptr<NumericDocValues>
  getNumericDocValues(const std::wstring &field)  override;

  std::shared_ptr<BinaryDocValues>
  getBinaryDocValues(const std::wstring &field)  override;

  std::shared_ptr<SortedDocValues>
  getSortedDocValues(const std::wstring &field)  override;

  std::shared_ptr<SortedNumericDocValues> getSortedNumericDocValues(
      const std::wstring &field)  override;

  std::shared_ptr<SortedSetDocValues>
  getSortedSetDocValues(const std::wstring &field)  override;

  std::shared_ptr<NumericDocValues>
  getNormValues(const std::wstring &field)  override;

  std::shared_ptr<LeafMetaData> getMetaData() override;

  void checkIntegrity()  override;

  /** Returns the wrapped {@link LeafReader}. */
  virtual std::shared_ptr<LeafReader> getDelegate();

protected:
  std::shared_ptr<FilterLeafReader> shared_from_this()
  {
    return std::static_pointer_cast<FilterLeafReader>(
        LeafReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
