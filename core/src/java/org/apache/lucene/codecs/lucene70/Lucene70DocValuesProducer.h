#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::lucene70
{
class NumericEntry;
}

namespace org::apache::lucene::codecs::lucene70
{
class BinaryEntry;
}
namespace org::apache::lucene::codecs::lucene70
{
class SortedEntry;
}
namespace org::apache::lucene::codecs::lucene70
{
class SortedSetEntry;
}
namespace org::apache::lucene::codecs::lucene70
{
class SortedNumericEntry;
}
namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::store
{
class ChecksumIndexInput;
}
namespace org::apache::lucene::codecs::lucene70
{
class TermsDictEntry;
}
namespace org::apache::lucene::util::packed
{
class DirectMonotonicReader;
}
namespace org::apache::lucene::util::packed
{
class Meta;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::codecs::lucene70
{
class IndexedDISI;
}
namespace org::apache::lucene::store
{
class RandomAccessInput;
}
namespace org::apache::lucene::util
{
class LongValues;
}
namespace org::apache::lucene::index
{
class BinaryDocValues;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::index
{
class SortedNumericDocValues;
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
namespace org::apache::lucene::codecs::lucene70
{

using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using SeekStatus = org::apache::lucene::index::TermsEnum::SeekStatus;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using LongValues = org::apache::lucene::util::LongValues;
using DirectMonotonicReader =
    org::apache::lucene::util::packed::DirectMonotonicReader;

/** reader for {@link Lucene70DocValuesFormat} */
class Lucene70DocValuesProducer final : public DocValuesProducer
{
  GET_CLASS_NAME(Lucene70DocValuesProducer)
private:
  const std::unordered_map<std::wstring, std::shared_ptr<NumericEntry>>
      numerics =
          std::unordered_map<std::wstring, std::shared_ptr<NumericEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<BinaryEntry>>
      binaries =
          std::unordered_map<std::wstring, std::shared_ptr<BinaryEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<SortedEntry>> sorted =
      std::unordered_map<std::wstring, std::shared_ptr<SortedEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<SortedSetEntry>>
      sortedSets =
          std::unordered_map<std::wstring, std::shared_ptr<SortedSetEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<SortedNumericEntry>>
      sortedNumerics =
          std::unordered_map<std::wstring,
                             std::shared_ptr<SortedNumericEntry>>();
  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t ramBytesUsed_ = 0;
  const std::shared_ptr<IndexInput> data;
  const int maxDoc;

  /** expert: instantiates a new reader */
public:
  Lucene70DocValuesProducer(
      std::shared_ptr<SegmentReadState> state, const std::wstring &dataCodec,
      const std::wstring &dataExtension, const std::wstring &metaCodec,
      const std::wstring &metaExtension) ;

private:
  void readFields(std::shared_ptr<ChecksumIndexInput> meta,
                  std::shared_ptr<FieldInfos> infos) ;

  std::shared_ptr<NumericEntry>
  readNumeric(std::shared_ptr<ChecksumIndexInput> meta) ;

  void readNumeric(std::shared_ptr<ChecksumIndexInput> meta,
                   std::shared_ptr<NumericEntry> entry) ;

  std::shared_ptr<BinaryEntry>
  readBinary(std::shared_ptr<ChecksumIndexInput> meta) ;

  std::shared_ptr<SortedEntry>
  readSorted(std::shared_ptr<ChecksumIndexInput> meta) ;

  std::shared_ptr<SortedSetEntry>
  readSortedSet(std::shared_ptr<ChecksumIndexInput> meta) ;

  static void
  readTermDict(std::shared_ptr<ChecksumIndexInput> meta,
               std::shared_ptr<TermsDictEntry> entry) ;

  std::shared_ptr<SortedNumericEntry> readSortedNumeric(
      std::shared_ptr<ChecksumIndexInput> meta) ;

public:
  virtual ~Lucene70DocValuesProducer();

private:
  class NumericEntry : public std::enable_shared_from_this<NumericEntry>
  {
    GET_CLASS_NAME(NumericEntry)
  public:
    std::deque<int64_t> table;
    int blockShift = 0;
    char bitsPerValue = 0;
    int64_t docsWithFieldOffset = 0;
    int64_t docsWithFieldLength = 0;
    int64_t numValues = 0;
    int64_t minValue = 0;
    int64_t gcd = 0;
    int64_t valuesOffset = 0;
    int64_t valuesLength = 0;
  };

private:
  class BinaryEntry : public std::enable_shared_from_this<BinaryEntry>
  {
    GET_CLASS_NAME(BinaryEntry)
  public:
    int64_t dataOffset = 0;
    int64_t dataLength = 0;
    int64_t docsWithFieldOffset = 0;
    int64_t docsWithFieldLength = 0;
    int numDocsWithField = 0;
    int minLength = 0;
    int maxLength = 0;
    int64_t addressesOffset = 0;
    int64_t addressesLength = 0;
    std::shared_ptr<DirectMonotonicReader::Meta> addressesMeta;
  };

private:
  class TermsDictEntry : public std::enable_shared_from_this<TermsDictEntry>
  {
    GET_CLASS_NAME(TermsDictEntry)
  public:
    int64_t termsDictSize = 0;
    int termsDictBlockShift = 0;
    std::shared_ptr<DirectMonotonicReader::Meta> termsAddressesMeta;
    int maxTermLength = 0;
    int64_t termsDataOffset = 0;
    int64_t termsDataLength = 0;
    int64_t termsAddressesOffset = 0;
    int64_t termsAddressesLength = 0;
    int termsDictIndexShift = 0;
    std::shared_ptr<DirectMonotonicReader::Meta> termsIndexAddressesMeta;
    int64_t termsIndexOffset = 0;
    int64_t termsIndexLength = 0;
    int64_t termsIndexAddressesOffset = 0;
    int64_t termsIndexAddressesLength = 0;
  };

private:
  class SortedEntry : public TermsDictEntry
  {
    GET_CLASS_NAME(SortedEntry)
  public:
    int64_t docsWithFieldOffset = 0;
    int64_t docsWithFieldLength = 0;
    int numDocsWithField = 0;
    char bitsPerValue = 0;
    int64_t ordsOffset = 0;
    int64_t ordsLength = 0;

  protected:
    std::shared_ptr<SortedEntry> shared_from_this()
    {
      return std::static_pointer_cast<SortedEntry>(
          TermsDictEntry::shared_from_this());
    }
  };

private:
  class SortedSetEntry : public TermsDictEntry
  {
    GET_CLASS_NAME(SortedSetEntry)
  public:
    std::shared_ptr<SortedEntry> singleValueEntry;
    int64_t docsWithFieldOffset = 0;
    int64_t docsWithFieldLength = 0;
    int numDocsWithField = 0;
    char bitsPerValue = 0;
    int64_t ordsOffset = 0;
    int64_t ordsLength = 0;
    std::shared_ptr<DirectMonotonicReader::Meta> addressesMeta;
    int64_t addressesOffset = 0;
    int64_t addressesLength = 0;

  protected:
    std::shared_ptr<SortedSetEntry> shared_from_this()
    {
      return std::static_pointer_cast<SortedSetEntry>(
          TermsDictEntry::shared_from_this());
    }
  };

private:
  class SortedNumericEntry : public NumericEntry
  {
    GET_CLASS_NAME(SortedNumericEntry)
  public:
    int numDocsWithField = 0;
    std::shared_ptr<DirectMonotonicReader::Meta> addressesMeta;
    int64_t addressesOffset = 0;
    int64_t addressesLength = 0;

  protected:
    std::shared_ptr<SortedNumericEntry> shared_from_this()
    {
      return std::static_pointer_cast<SortedNumericEntry>(
          NumericEntry::shared_from_this());
    }
  };

public:
  int64_t ramBytesUsed() override;

  std::shared_ptr<NumericDocValues>
  getNumeric(std::shared_ptr<FieldInfo> field)  override;

private:
  class DenseNumericDocValues : public NumericDocValues
  {
    GET_CLASS_NAME(DenseNumericDocValues)

  public:
    const int maxDoc;
    int doc = -1;

    DenseNumericDocValues(int maxDoc);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target) override;

    int64_t cost() override;

  protected:
    std::shared_ptr<DenseNumericDocValues> shared_from_this()
    {
      return std::static_pointer_cast<DenseNumericDocValues>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };

private:
  class SparseNumericDocValues : public NumericDocValues
  {
    GET_CLASS_NAME(SparseNumericDocValues)

  public:
    const std::shared_ptr<IndexedDISI> disi;

    SparseNumericDocValues(std::shared_ptr<IndexedDISI> disi);

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SparseNumericDocValues> shared_from_this()
    {
      return std::static_pointer_cast<SparseNumericDocValues>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };

private:
  std::shared_ptr<NumericDocValues>
  getNumeric(std::shared_ptr<NumericEntry> entry) ;

private:
  class DenseNumericDocValuesAnonymousInnerClass : public DenseNumericDocValues
  {
    GET_CLASS_NAME(DenseNumericDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::
                        Lucene70DocValuesProducer::NumericEntry>
        entry;

  public:
    DenseNumericDocValuesAnonymousInnerClass(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance, int maxDoc,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::
                            Lucene70DocValuesProducer::NumericEntry>
            entry);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<DenseNumericDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DenseNumericDocValuesAnonymousInnerClass>(
          DenseNumericDocValues::shared_from_this());
    }
  };

private:
  class DenseNumericDocValuesAnonymousInnerClass2 : public DenseNumericDocValues
  {
    GET_CLASS_NAME(DenseNumericDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<RandomAccessInput> slice;
    int shift = 0;
    int64_t mul = 0;
    int mask = 0;

  public:
    DenseNumericDocValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance, int maxDoc,
        std::shared_ptr<RandomAccessInput> slice, int shift, int64_t mul,
        int mask);

    int block = 0;
    int64_t delta = 0;
    int64_t offset = 0;
    int64_t blockEndOffset = 0;
    std::shared_ptr<LongValues> values;

    int64_t longValue()  override;

  protected:
    std::shared_ptr<DenseNumericDocValuesAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DenseNumericDocValuesAnonymousInnerClass2>(
          DenseNumericDocValues::shared_from_this());
    }
  };

private:
  class DenseNumericDocValuesAnonymousInnerClass3 : public DenseNumericDocValues
  {
    GET_CLASS_NAME(DenseNumericDocValuesAnonymousInnerClass3)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> values;
    std::deque<int64_t> table;

  public:
    DenseNumericDocValuesAnonymousInnerClass3(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance, int maxDoc,
        std::shared_ptr<LongValues> values, std::deque<int64_t> &table);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<DenseNumericDocValuesAnonymousInnerClass3>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DenseNumericDocValuesAnonymousInnerClass3>(
          DenseNumericDocValues::shared_from_this());
    }
  };

private:
  class DenseNumericDocValuesAnonymousInnerClass4 : public DenseNumericDocValues
  {
    GET_CLASS_NAME(DenseNumericDocValuesAnonymousInnerClass4)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> values;
    int64_t mul = 0;
    int64_t delta = 0;

  public:
    DenseNumericDocValuesAnonymousInnerClass4(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance, int maxDoc,
        std::shared_ptr<LongValues> values, int64_t mul, int64_t delta);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<DenseNumericDocValuesAnonymousInnerClass4>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DenseNumericDocValuesAnonymousInnerClass4>(
          DenseNumericDocValues::shared_from_this());
    }
  };

private:
  class SparseNumericDocValuesAnonymousInnerClass
      : public SparseNumericDocValues
  {
    GET_CLASS_NAME(SparseNumericDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::
                        Lucene70DocValuesProducer::NumericEntry>
        entry;

  public:
    SparseNumericDocValuesAnonymousInnerClass(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::
                            Lucene70DocValuesProducer::NumericEntry>
            entry);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<SparseNumericDocValuesAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SparseNumericDocValuesAnonymousInnerClass>(
          SparseNumericDocValues::shared_from_this());
    }
  };

private:
  class SparseNumericDocValuesAnonymousInnerClass2
      : public SparseNumericDocValues
  {
    GET_CLASS_NAME(SparseNumericDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi;
    std::shared_ptr<RandomAccessInput> slice;
    int shift = 0;
    int64_t mul = 0;
    int mask = 0;

  public:
    SparseNumericDocValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI>
            disi,
        std::shared_ptr<RandomAccessInput> slice, int shift, int64_t mul,
        int mask);

    int block = 0;
    int64_t delta = 0;
    int64_t offset = 0;
    int64_t blockEndOffset = 0;
    std::shared_ptr<LongValues> values;

    int64_t longValue()  override;

  protected:
    std::shared_ptr<SparseNumericDocValuesAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SparseNumericDocValuesAnonymousInnerClass2>(
          SparseNumericDocValues::shared_from_this());
    }
  };

private:
  class SparseNumericDocValuesAnonymousInnerClass3
      : public SparseNumericDocValues
  {
    GET_CLASS_NAME(SparseNumericDocValuesAnonymousInnerClass3)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi;
    std::shared_ptr<LongValues> values;
    std::deque<int64_t> table;

  public:
    SparseNumericDocValuesAnonymousInnerClass3(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI>
            disi,
        std::shared_ptr<LongValues> values, std::deque<int64_t> &table);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<SparseNumericDocValuesAnonymousInnerClass3>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SparseNumericDocValuesAnonymousInnerClass3>(
          SparseNumericDocValues::shared_from_this());
    }
  };

private:
  class SparseNumericDocValuesAnonymousInnerClass4
      : public SparseNumericDocValues
  {
    GET_CLASS_NAME(SparseNumericDocValuesAnonymousInnerClass4)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi;
    std::shared_ptr<LongValues> values;
    int64_t mul = 0;
    int64_t delta = 0;

  public:
    SparseNumericDocValuesAnonymousInnerClass4(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI>
            disi,
        std::shared_ptr<LongValues> values, int64_t mul, int64_t delta);

    int64_t longValue()  override;

  protected:
    std::shared_ptr<SparseNumericDocValuesAnonymousInnerClass4>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SparseNumericDocValuesAnonymousInnerClass4>(
          SparseNumericDocValues::shared_from_this());
    }
  };

private:
  std::shared_ptr<LongValues>
  getNumericValues(std::shared_ptr<NumericEntry> entry) ;

private:
  class LongValuesAnonymousInnerClass : public LongValues
  {
    GET_CLASS_NAME(LongValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::
                        Lucene70DocValuesProducer::NumericEntry>
        entry;

  public:
    LongValuesAnonymousInnerClass(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::
                            Lucene70DocValuesProducer::NumericEntry>
            entry);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<LongValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesAnonymousInnerClass>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

private:
  class LongValuesAnonymousInnerClass2 : public LongValues
  {
    GET_CLASS_NAME(LongValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<RandomAccessInput> slice;
    int shift = 0;
    int64_t mul = 0;
    int64_t mask = 0;

  public:
    LongValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<RandomAccessInput> slice, int shift, int64_t mul,
        int64_t mask);

    int64_t block = 0;
    int64_t delta = 0;
    int64_t offset = 0;
    int64_t blockEndOffset = 0;
    std::shared_ptr<LongValues> values;

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<LongValuesAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesAnonymousInnerClass2>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

private:
  class LongValuesAnonymousInnerClass3 : public LongValues
  {
    GET_CLASS_NAME(LongValuesAnonymousInnerClass3)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> values;
    std::deque<int64_t> table;

  public:
    LongValuesAnonymousInnerClass3(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<LongValues> values, std::deque<int64_t> &table);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<LongValuesAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesAnonymousInnerClass3>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

private:
  class LongValuesAnonymousInnerClass4 : public LongValues
  {
    GET_CLASS_NAME(LongValuesAnonymousInnerClass4)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> values;
    int64_t gcd = 0;
    int64_t minValue = 0;

  public:
    LongValuesAnonymousInnerClass4(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<LongValues> values, int64_t gcd, int64_t minValue);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<LongValuesAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesAnonymousInnerClass4>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

private:
  class LongValuesAnonymousInnerClass5 : public LongValues
  {
    GET_CLASS_NAME(LongValuesAnonymousInnerClass5)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> values;
    int64_t minValue = 0;

  public:
    LongValuesAnonymousInnerClass5(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<LongValues> values, int64_t minValue);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<LongValuesAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesAnonymousInnerClass5>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

private:
  class DenseBinaryDocValues : public BinaryDocValues
  {
    GET_CLASS_NAME(DenseBinaryDocValues)

  public:
    const int maxDoc;
    int doc = -1;

    DenseBinaryDocValues(int maxDoc);

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

  protected:
    std::shared_ptr<DenseBinaryDocValues> shared_from_this()
    {
      return std::static_pointer_cast<DenseBinaryDocValues>(
          org.apache.lucene.index.BinaryDocValues::shared_from_this());
    }
  };

private:
  class SparseBinaryDocValues : public BinaryDocValues
  {
    GET_CLASS_NAME(SparseBinaryDocValues)

  public:
    const std::shared_ptr<IndexedDISI> disi;

    SparseBinaryDocValues(std::shared_ptr<IndexedDISI> disi);

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

  protected:
    std::shared_ptr<SparseBinaryDocValues> shared_from_this()
    {
      return std::static_pointer_cast<SparseBinaryDocValues>(
          org.apache.lucene.index.BinaryDocValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<BinaryDocValues>
  getBinary(std::shared_ptr<FieldInfo> field)  override;

private:
  class DenseBinaryDocValuesAnonymousInnerClass : public DenseBinaryDocValues
  {
    GET_CLASS_NAME(DenseBinaryDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<IndexInput> bytesSlice;
    int length = 0;

  public:
    DenseBinaryDocValuesAnonymousInnerClass(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance, int maxDoc,
        std::shared_ptr<IndexInput> bytesSlice, int length);

    const std::shared_ptr<BytesRef> bytes;

    std::shared_ptr<BytesRef> binaryValue()  override;

  protected:
    std::shared_ptr<DenseBinaryDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DenseBinaryDocValuesAnonymousInnerClass>(
          DenseBinaryDocValues::shared_from_this());
    }
  };

private:
  class DenseBinaryDocValuesAnonymousInnerClass2 : public DenseBinaryDocValues
  {
    GET_CLASS_NAME(DenseBinaryDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::
                        Lucene70DocValuesProducer::BinaryEntry>
        entry;
    std::shared_ptr<IndexInput> bytesSlice;
    std::shared_ptr<LongValues> addresses;

  public:
    DenseBinaryDocValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance, int maxDoc,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::
                            Lucene70DocValuesProducer::BinaryEntry>
            entry,
        std::shared_ptr<IndexInput> bytesSlice,
        std::shared_ptr<LongValues> addresses);

    const std::shared_ptr<BytesRef> bytes;

    std::shared_ptr<BytesRef> binaryValue()  override;

  protected:
    std::shared_ptr<DenseBinaryDocValuesAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<DenseBinaryDocValuesAnonymousInnerClass2>(
          DenseBinaryDocValues::shared_from_this());
    }
  };

private:
  class SparseBinaryDocValuesAnonymousInnerClass : public SparseBinaryDocValues
  {
    GET_CLASS_NAME(SparseBinaryDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<IndexInput> bytesSlice;
    std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi;
    int length = 0;

  public:
    SparseBinaryDocValuesAnonymousInnerClass(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<IndexInput> bytesSlice,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI>
            disi,
        int length);

    const std::shared_ptr<BytesRef> bytes;

    std::shared_ptr<BytesRef> binaryValue()  override;

  protected:
    std::shared_ptr<SparseBinaryDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SparseBinaryDocValuesAnonymousInnerClass>(
          SparseBinaryDocValues::shared_from_this());
    }
  };

private:
  class SparseBinaryDocValuesAnonymousInnerClass2 : public SparseBinaryDocValues
  {
    GET_CLASS_NAME(SparseBinaryDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene70::
                        Lucene70DocValuesProducer::BinaryEntry>
        entry;
    std::shared_ptr<IndexInput> bytesSlice;
    std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi;
    std::shared_ptr<LongValues> addresses;

  public:
    SparseBinaryDocValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::
                            Lucene70DocValuesProducer::BinaryEntry>
            entry,
        std::shared_ptr<IndexInput> bytesSlice,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI>
            disi,
        std::shared_ptr<LongValues> addresses);

    const std::shared_ptr<BytesRef> bytes;

    std::shared_ptr<BytesRef> binaryValue()  override;

  protected:
    std::shared_ptr<SparseBinaryDocValuesAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SparseBinaryDocValuesAnonymousInnerClass2>(
          SparseBinaryDocValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<SortedDocValues>
  getSorted(std::shared_ptr<FieldInfo> field)  override;

private:
  std::shared_ptr<SortedDocValues>
  getSorted(std::shared_ptr<SortedEntry> entry) ;

private:
  class LongValuesAnonymousInnerClass : public LongValues
  {
    GET_CLASS_NAME(LongValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

  public:
    LongValuesAnonymousInnerClass(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<LongValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesAnonymousInnerClass>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

private:
  class BaseSortedDocValuesAnonymousInnerClass : public BaseSortedDocValues
  {
    GET_CLASS_NAME(BaseSortedDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> ords;

  public:
    BaseSortedDocValuesAnonymousInnerClass(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::
                            Lucene70DocValuesProducer::SortedEntry>
            entry,
        std::shared_ptr<IndexInput> data, std::shared_ptr<LongValues> ords);

    int doc = 0;

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target) override;

    int ordValue() override;

  protected:
    std::shared_ptr<BaseSortedDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<BaseSortedDocValuesAnonymousInnerClass>(
          BaseSortedDocValues::shared_from_this());
    }
  };

private:
  class BaseSortedDocValuesAnonymousInnerClass2 : public BaseSortedDocValues
  {
    GET_CLASS_NAME(BaseSortedDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> ords;
    std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi;

  public:
    BaseSortedDocValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::
                            Lucene70DocValuesProducer::SortedEntry>
            entry,
        std::shared_ptr<IndexInput> data, std::shared_ptr<LongValues> ords,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI>
            disi);

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int ordValue() override;

  protected:
    std::shared_ptr<BaseSortedDocValuesAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<BaseSortedDocValuesAnonymousInnerClass2>(
          BaseSortedDocValues::shared_from_this());
    }
  };

private:
  class BaseSortedDocValues : public SortedDocValues
  {
    GET_CLASS_NAME(BaseSortedDocValues)

  public:
    const std::shared_ptr<SortedEntry> entry;
    const std::shared_ptr<IndexInput> data;
    // C++ NOTE: Fields cannot have the same name as methods:
    const std::shared_ptr<TermsEnum> termsEnum_;

    BaseSortedDocValues(std::shared_ptr<SortedEntry> entry,
                        std::shared_ptr<IndexInput> data) ;

    int getValueCount() override;

    std::shared_ptr<BytesRef> lookupOrd(int ord)  override;

    int lookupTerm(std::shared_ptr<BytesRef> key)  override;

    std::shared_ptr<TermsEnum> termsEnum()  override;

  protected:
    std::shared_ptr<BaseSortedDocValues> shared_from_this()
    {
      return std::static_pointer_cast<BaseSortedDocValues>(
          org.apache.lucene.index.SortedDocValues::shared_from_this());
    }
  };

private:
  class BaseSortedSetDocValues : public SortedSetDocValues
  {
    GET_CLASS_NAME(BaseSortedSetDocValues)

  public:
    const std::shared_ptr<SortedSetEntry> entry;
    const std::shared_ptr<IndexInput> data;
    // C++ NOTE: Fields cannot have the same name as methods:
    const std::shared_ptr<TermsEnum> termsEnum_;

    BaseSortedSetDocValues(std::shared_ptr<SortedSetEntry> entry,
                           std::shared_ptr<IndexInput> data) ;

    int64_t getValueCount() override;

    std::shared_ptr<BytesRef>
    lookupOrd(int64_t ord)  override;

    int64_t
    lookupTerm(std::shared_ptr<BytesRef> key)  override;

    std::shared_ptr<TermsEnum> termsEnum()  override;

  protected:
    std::shared_ptr<BaseSortedSetDocValues> shared_from_this()
    {
      return std::static_pointer_cast<BaseSortedSetDocValues>(
          org.apache.lucene.index.SortedSetDocValues::shared_from_this());
    }
  };

private:
  class TermsDict : public TermsEnum
  {
    GET_CLASS_NAME(TermsDict)

  public:
    const std::shared_ptr<TermsDictEntry> entry;
    const std::shared_ptr<LongValues> blockAddresses;
    const std::shared_ptr<IndexInput> bytes;
    const int64_t blockMask;
    const std::shared_ptr<LongValues> indexAddresses;
    const std::shared_ptr<IndexInput> indexBytes;
    // C++ NOTE: Fields cannot have the same name as methods:
    const std::shared_ptr<BytesRef> term_;
    // C++ NOTE: Fields cannot have the same name as methods:
    int64_t ord_ = -1;

    TermsDict(std::shared_ptr<TermsDictEntry> entry,
              std::shared_ptr<IndexInput> data) ;

    std::shared_ptr<BytesRef> next()  override;

    void seekExact(int64_t ord)  override;

  private:
    std::shared_ptr<BytesRef>
    getTermFromIndex(int64_t index) ;

    int64_t seekTermsIndex(std::shared_ptr<BytesRef> text) ;

    std::shared_ptr<BytesRef>
    getFirstTermFromBlock(int64_t block) ;

    int64_t seekBlock(std::shared_ptr<BytesRef> text) ;

  public:
    TermsEnum::SeekStatus
    seekCeil(std::shared_ptr<BytesRef> text)  override;

    std::shared_ptr<BytesRef> term()  override;

    int64_t ord()  override;

    int64_t totalTermFreq()  override;

    std::shared_ptr<PostingsEnum>
    postings(std::shared_ptr<PostingsEnum> reuse,
             int flags)  override;

    int docFreq()  override;

  protected:
    std::shared_ptr<TermsDict> shared_from_this()
    {
      return std::static_pointer_cast<TermsDict>(
          org.apache.lucene.index.TermsEnum::shared_from_this());
    }
  };

public:
  std::shared_ptr<SortedNumericDocValues> getSortedNumeric(
      std::shared_ptr<FieldInfo> field)  override;

private:
  class SortedNumericDocValuesAnonymousInnerClass
      : public SortedNumericDocValues
  {
    GET_CLASS_NAME(SortedNumericDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> addresses;
    std::shared_ptr<LongValues> values;

  public:
    SortedNumericDocValuesAnonymousInnerClass(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<LongValues> addresses,
        std::shared_ptr<LongValues> values);

    int doc = 0;
    int64_t start = 0, end = 0;
    int count = 0;

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t nextValue()  override;

    int docValueCount() override;

  protected:
    std::shared_ptr<SortedNumericDocValuesAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SortedNumericDocValuesAnonymousInnerClass>(
          org.apache.lucene.index.SortedNumericDocValues::shared_from_this());
    }
  };

private:
  class SortedNumericDocValuesAnonymousInnerClass2
      : public SortedNumericDocValues
  {
    GET_CLASS_NAME(SortedNumericDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> addresses;
    std::shared_ptr<LongValues> values;
    std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi;

  public:
    SortedNumericDocValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<LongValues> addresses,
        std::shared_ptr<LongValues> values,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI>
            disi);

    bool set = false;
    int64_t start = 0, end = 0;
    int count = 0;

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t nextValue()  override;

    int docValueCount() override;

  private:
    void set();

  protected:
    std::shared_ptr<SortedNumericDocValuesAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SortedNumericDocValuesAnonymousInnerClass2>(
          org.apache.lucene.index.SortedNumericDocValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<SortedSetDocValues>
  getSortedSet(std::shared_ptr<FieldInfo> field)  override;

private:
  class BaseSortedSetDocValuesAnonymousInnerClass
      : public BaseSortedSetDocValues
  {
    GET_CLASS_NAME(BaseSortedSetDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> ords;
    std::shared_ptr<LongValues> addresses;

  public:
    BaseSortedSetDocValuesAnonymousInnerClass(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::
                            Lucene70DocValuesProducer::SortedSetEntry>
            entry,
        std::shared_ptr<IndexInput> data, std::shared_ptr<LongValues> ords,
        std::shared_ptr<LongValues> addresses);

    int doc = 0;
    int64_t start = 0;
    int64_t end = 0;

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t nextOrd()  override;

  protected:
    std::shared_ptr<BaseSortedSetDocValuesAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          BaseSortedSetDocValuesAnonymousInnerClass>(
          BaseSortedSetDocValues::shared_from_this());
    }
  };

private:
  class BaseSortedSetDocValuesAnonymousInnerClass2
      : public BaseSortedSetDocValues
  {
    GET_CLASS_NAME(BaseSortedSetDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene70DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> ords;
    std::shared_ptr<LongValues> addresses;
    std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI> disi;

  public:
    BaseSortedSetDocValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene70DocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::
                            Lucene70DocValuesProducer::SortedSetEntry>
            entry,
        std::shared_ptr<IndexInput> data, std::shared_ptr<LongValues> ords,
        std::shared_ptr<LongValues> addresses,
        std::shared_ptr<org::apache::lucene::codecs::lucene70::IndexedDISI>
            disi);

    bool set = false;
    int64_t start = 0;
    int64_t end = 0;

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t nextOrd()  override;

  protected:
    std::shared_ptr<BaseSortedSetDocValuesAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          BaseSortedSetDocValuesAnonymousInnerClass2>(
          BaseSortedSetDocValues::shared_from_this());
    }
  };

public:
  void checkIntegrity()  override;

protected:
  std::shared_ptr<Lucene70DocValuesProducer> shared_from_this()
  {
    return std::static_pointer_cast<Lucene70DocValuesProducer>(
        org.apache.lucene.codecs.DocValuesProducer::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene70
