#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Bits.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/LongValues.h"
#include "Lucene54DocValuesConsumer.h"
#include "Lucene54DocValuesFormat.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/lucene54/NumericEntry.h"

#include  "core/src/java/org/apache/lucene/codecs/lucene54/BinaryEntry.h"
#include  "core/src/java/org/apache/lucene/codecs/lucene54/SortedSetEntry.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/util/packed/MonotonicBlockPackedReader.h"
#include  "core/src/java/org/apache/lucene/codecs/lucene54/ReverseTermsIndex.h"
#include  "core/src/java/org/apache/lucene/util/packed/DirectMonotonicReader.h"
#include  "core/src/java/org/apache/lucene/util/packed/Meta.h"
#include  "core/src/java/org/apache/lucene/util/LongValues.h"
#include  "core/src/java/org/apache/lucene/store/RandomAccessInput.h"
#include  "core/src/java/org/apache/lucene/util/PagedBytes.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/LegacyBinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/codecs/lucene54/LongBinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include  "core/src/java/org/apache/lucene/util/Reader.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"
#include  "core/src/java/org/apache/lucene/codecs/lucene54/CompressedBinaryTermsEnum.h"

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
namespace org::apache::lucene::codecs::lucene54
{

using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using NumberType = org::apache::lucene::codecs::lucene54::
    Lucene54DocValuesConsumer::NumberType;
using namespace org::apache::lucene::index;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using LongValues = org::apache::lucene::util::LongValues;
using PagedBytes = org::apache::lucene::util::PagedBytes;
using DirectMonotonicReader =
    org::apache::lucene::util::packed::DirectMonotonicReader;
using MonotonicBlockPackedReader =
    org::apache::lucene::util::packed::MonotonicBlockPackedReader;

using namespace org::apache::lucene::codecs::lucene54;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.codecs.lucene54.Lucene54DocValuesFormat.*;

/** reader for {@link Lucene54DocValuesFormat} */
class Lucene54DocValuesProducer final : public DocValuesProducer
{
  GET_CLASS_NAME(Lucene54DocValuesProducer)
private:
  const std::unordered_map<std::wstring, std::shared_ptr<NumericEntry>>
      numerics =
          std::unordered_map<std::wstring, std::shared_ptr<NumericEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<BinaryEntry>>
      binaries =
          std::unordered_map<std::wstring, std::shared_ptr<BinaryEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<SortedSetEntry>>
      sortedSets =
          std::unordered_map<std::wstring, std::shared_ptr<SortedSetEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<SortedSetEntry>>
      sortedNumerics =
          std::unordered_map<std::wstring, std::shared_ptr<SortedSetEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<NumericEntry>> ords =
      std::unordered_map<std::wstring, std::shared_ptr<NumericEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<NumericEntry>>
      ordIndexes =
          std::unordered_map<std::wstring, std::shared_ptr<NumericEntry>>();
  const int numFields;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<AtomicLong> ramBytesUsed_;
  const std::shared_ptr<IndexInput> data;
  const int maxDoc;

  // memory-resident structures
  const std::unordered_map<std::wstring,
                           std::shared_ptr<MonotonicBlockPackedReader>>
      addressInstances =
          std::unordered_map<std::wstring,
                             std::shared_ptr<MonotonicBlockPackedReader>>();
  const std::unordered_map<std::wstring, std::shared_ptr<ReverseTermsIndex>>
      reverseIndexInstances =
          std::unordered_map<std::wstring,
                             std::shared_ptr<ReverseTermsIndex>>();
  const std::unordered_map<std::wstring,
                           std::shared_ptr<DirectMonotonicReader::Meta>>
      directAddressesMeta =
          std::unordered_map<std::wstring,
                             std::shared_ptr<DirectMonotonicReader::Meta>>();

  const bool merging;

  // clone for merge: when merging we don't do any instances.put()s
public:
  Lucene54DocValuesProducer(
      std::shared_ptr<Lucene54DocValuesProducer> original) ;

  /** expert: instantiates a new reader */
  Lucene54DocValuesProducer(
      std::shared_ptr<SegmentReadState> state, const std::wstring &dataCodec,
      const std::wstring &dataExtension, const std::wstring &metaCodec,
      const std::wstring &metaExtension) ;

private:
  void readSortedField(std::shared_ptr<FieldInfo> info,
                       std::shared_ptr<IndexInput> meta) ;

  void readSortedSetFieldWithAddresses(
      std::shared_ptr<FieldInfo> info,
      std::shared_ptr<IndexInput> meta) ;

  void readSortedSetFieldWithTable(
      std::shared_ptr<FieldInfo> info,
      std::shared_ptr<IndexInput> meta) ;

  int readFields(std::shared_ptr<IndexInput> meta,
                 std::shared_ptr<FieldInfos> infos) ;

  std::shared_ptr<NumericEntry>
  readNumericEntry(std::shared_ptr<FieldInfo> info,
                   std::shared_ptr<IndexInput> meta) ;

  std::shared_ptr<BinaryEntry>
  readBinaryEntry(std::shared_ptr<FieldInfo> info,
                  std::shared_ptr<IndexInput> meta) ;

public:
  std::shared_ptr<SortedSetEntry>
  readSortedSetEntry(std::shared_ptr<IndexInput> meta) ;

  std::shared_ptr<NumericDocValues>
  getNumeric(std::shared_ptr<FieldInfo> field)  override;

private:
  class NumericDocValuesAnonymousInnerClass : public NumericDocValues
  {
    GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> values;

  public:
    NumericDocValuesAnonymousInnerClass(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        std::shared_ptr<LongValues> values);

  private:
    int docID = 0;

  public:
    int docID() override;

    int nextDoc() override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int64_t longValue() override;

  protected:
    std::shared_ptr<NumericDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };

private:
  class NumericDocValuesAnonymousInnerClass2 : public NumericDocValues
  {
    GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    std::shared_ptr<Bits> docsWithField;
    std::shared_ptr<LongValues> values;

  public:
    NumericDocValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        std::shared_ptr<Bits> docsWithField,
        std::shared_ptr<LongValues> values);

    int doc = 0;
    int64_t value = 0;

    int64_t longValue()  override;

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<NumericDocValuesAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass2>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };

public:
  int64_t ramBytesUsed() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  void checkIntegrity()  override;

  virtual std::wstring toString();

  std::shared_ptr<LongValues>
  getNumeric(std::shared_ptr<NumericEntry> entry) ;

private:
  class LongValuesAnonymousInnerClass : public LongValues
  {
    GET_CLASS_NAME(LongValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    int64_t constant = 0;
    std::shared_ptr<Bits> live;

  public:
    LongValuesAnonymousInnerClass(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        int64_t constant, std::shared_ptr<Bits> live);

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
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    int64_t delta = 0;
    std::shared_ptr<LongValues> values;

  public:
    LongValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        int64_t delta, std::shared_ptr<LongValues> values);

    int64_t get(int64_t id) override;

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
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    int64_t min = 0;
    int64_t mult = 0;
    std::shared_ptr<LongValues> quotientReader;

  public:
    LongValuesAnonymousInnerClass3(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance, int64_t min,
        int64_t mult, std::shared_ptr<LongValues> quotientReader);

    int64_t get(int64_t id) override;

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
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    std::deque<int64_t> table;
    std::shared_ptr<LongValues> ords;

  public:
    LongValuesAnonymousInnerClass4(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        std::deque<int64_t> &table, std::shared_ptr<LongValues> ords);

    int64_t get(int64_t id) override;

  protected:
    std::shared_ptr<LongValuesAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesAnonymousInnerClass4>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class SparseNumericDocValues final : public NumericDocValues
  {
    GET_CLASS_NAME(SparseNumericDocValues)

  public:
    const int docIDsLength;
    const std::shared_ptr<LongValues> docIds, values;

    int index = 0, doc = 0;

    SparseNumericDocValues(int docIDsLength, std::shared_ptr<LongValues> docIDs,
                           std::shared_ptr<LongValues> values);

    void reset();

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t longValue() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SparseNumericDocValues> shared_from_this()
    {
      return std::static_pointer_cast<SparseNumericDocValues>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };

public:
  class SparseNumericDocValuesRandomAccessWrapper : public LongValues
  {
    GET_CLASS_NAME(SparseNumericDocValuesRandomAccessWrapper)

  public:
    const std::shared_ptr<SparseNumericDocValues> values;
    const int64_t missingValue;

    SparseNumericDocValuesRandomAccessWrapper(
        std::shared_ptr<SparseNumericDocValues> values, int64_t missingValue);

    int64_t get(int64_t longIndex) override;

  protected:
    std::shared_ptr<SparseNumericDocValuesRandomAccessWrapper>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SparseNumericDocValuesRandomAccessWrapper>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<LegacyBinaryDocValues>
  getLegacyBinary(std::shared_ptr<FieldInfo> field) ;

  std::shared_ptr<BinaryDocValues>
  getBinary(std::shared_ptr<FieldInfo> field)  override;

private:
  std::shared_ptr<LegacyBinaryDocValues>
  getFixedBinary(std::shared_ptr<FieldInfo> field,
                 std::shared_ptr<BinaryEntry> bytes) ;

private:
  class LongBinaryDocValuesAnonymousInnerClass : public LongBinaryDocValues
  {
    GET_CLASS_NAME(LongBinaryDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    std::shared_ptr<IndexInput> data;
    std::shared_ptr<BytesRef> term;
    std::deque<char> buffer;
    int length = 0;

  public:
    LongBinaryDocValuesAnonymousInnerClass(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        std::shared_ptr<IndexInput> data, std::shared_ptr<BytesRef> term,
        std::deque<char> &buffer, int length);

    std::shared_ptr<BytesRef> get(int64_t id) override;

  protected:
    std::shared_ptr<LongBinaryDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LongBinaryDocValuesAnonymousInnerClass>(
          LongBinaryDocValues::shared_from_this());
    }
  };

private:
  std::shared_ptr<LegacyBinaryDocValues>
  getVariableBinary(std::shared_ptr<FieldInfo> field,
                    std::shared_ptr<BinaryEntry> bytes) ;

private:
  class LongBinaryDocValuesAnonymousInnerClass2 : public LongBinaryDocValues
  {
    GET_CLASS_NAME(LongBinaryDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> addresses;
    std::shared_ptr<IndexInput> data;
    std::shared_ptr<BytesRef> term;
    std::deque<char> buffer;

  public:
    LongBinaryDocValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        std::shared_ptr<LongValues> addresses, std::shared_ptr<IndexInput> data,
        std::shared_ptr<BytesRef> term, std::deque<char> &buffer);

    std::shared_ptr<BytesRef> get(int64_t id) override;

  protected:
    std::shared_ptr<LongBinaryDocValuesAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<LongBinaryDocValuesAnonymousInnerClass2>(
          LongBinaryDocValues::shared_from_this());
    }
  };

  /** returns an address instance for prefix-compressed binary values. */
private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<MonotonicBlockPackedReader>
  getIntervalInstance(std::shared_ptr<FieldInfo> field,
                      std::shared_ptr<BinaryEntry> bytes) ;

  /** returns a reverse lookup instance for prefix-compressed binary values. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<ReverseTermsIndex> getReverseIndexInstance(
      std::shared_ptr<FieldInfo> field,
      std::shared_ptr<BinaryEntry> bytes) ;

  std::shared_ptr<LegacyBinaryDocValues>
  getCompressedBinary(std::shared_ptr<FieldInfo> field,
                      std::shared_ptr<BinaryEntry> bytes) ;

public:
  std::shared_ptr<SortedDocValues>
  getSorted(std::shared_ptr<FieldInfo> field)  override;

private:
  class SortedDocValuesAnonymousInnerClass : public SortedDocValues
  {
    GET_CLASS_NAME(SortedDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    int valueCount = 0;
    std::shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues> binary;
    std::shared_ptr<org::apache::lucene::codecs::lucene54::
                        Lucene54DocValuesProducer::SparseNumericDocValues>
        sparseValues;

  public:
    SortedDocValuesAnonymousInnerClass(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        int valueCount,
        std::shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues>
            binary,
        std::shared_ptr<org::apache::lucene::codecs::lucene54::
                            Lucene54DocValuesProducer::SparseNumericDocValues>
            sparseValues);

    int ordValue() override;

    std::shared_ptr<BytesRef> lookupOrd(int ord) override;

    int getValueCount() override;

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SortedDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SortedDocValuesAnonymousInnerClass>(
          SortedDocValues::shared_from_this());
    }
  };

private:
  class SortedDocValuesAnonymousInnerClass2 : public SortedDocValues
  {
    GET_CLASS_NAME(SortedDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    int valueCount = 0;
    std::shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues> binary;
    std::shared_ptr<LongValues> ordinals;

  public:
    SortedDocValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        int valueCount,
        std::shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues>
            binary,
        std::shared_ptr<LongValues> ordinals);

  private:
    int docID = 0;
    int ord = 0;

  public:
    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int ordValue() override;

    int64_t cost() override;

    std::shared_ptr<BytesRef> lookupOrd(int ord) override;

    int getValueCount() override;

    int lookupTerm(std::shared_ptr<BytesRef> key)  override;

    std::shared_ptr<TermsEnum> termsEnum()  override;

  protected:
    std::shared_ptr<SortedDocValuesAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<SortedDocValuesAnonymousInnerClass2>(
          SortedDocValues::shared_from_this());
    }
  };

  /** returns an address instance for sortedset ordinal lists */
private:
  std::shared_ptr<LongValues>
  getOrdIndexInstance(std::shared_ptr<FieldInfo> field,
                      std::shared_ptr<NumericEntry> entry) ;

public:
  std::shared_ptr<SortedNumericDocValues> getSortedNumeric(
      std::shared_ptr<FieldInfo> field)  override;

private:
  class SortedNumericDocValuesAnonymousInnerClass
      : public SortedNumericDocValues
  {
    GET_CLASS_NAME(SortedNumericDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::lucene54::
                        Lucene54DocValuesProducer::SparseNumericDocValues>
        sparseValues;

  public:
    SortedNumericDocValuesAnonymousInnerClass(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::lucene54::
                            Lucene54DocValuesProducer::SparseNumericDocValues>
            sparseValues);

    int64_t nextValue()  override;

    int docValueCount() override;

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SortedNumericDocValuesAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SortedNumericDocValuesAnonymousInnerClass>(
          SortedNumericDocValues::shared_from_this());
    }
  };

private:
  class SortedNumericDocValuesAnonymousInnerClass2
      : public SortedNumericDocValues
  {
    GET_CLASS_NAME(SortedNumericDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> values;
    std::shared_ptr<Bits> docsWithField;

  public:
    SortedNumericDocValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        std::shared_ptr<LongValues> values,
        std::shared_ptr<Bits> docsWithField);

    int docID = 0;

    int docID() override;

    int nextDoc() override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int docValueCount() override;

    int64_t nextValue() override;

  protected:
    std::shared_ptr<SortedNumericDocValuesAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SortedNumericDocValuesAnonymousInnerClass2>(
          SortedNumericDocValues::shared_from_this());
    }
  };

private:
  class SortedNumericDocValuesAnonymousInnerClass3
      : public SortedNumericDocValues
  {
    GET_CLASS_NAME(SortedNumericDocValuesAnonymousInnerClass3)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> values;
    std::shared_ptr<LongValues> ordIndex;

  public:
    SortedNumericDocValuesAnonymousInnerClass3(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        std::shared_ptr<LongValues> values,
        std::shared_ptr<LongValues> ordIndex);

    int64_t startOffset = 0;
    int64_t endOffset = 0;
    int docID = 0;
    int64_t upto = 0;

    int docID() override;

    int nextDoc() override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int docValueCount() override;

    int64_t nextValue() override;

  protected:
    std::shared_ptr<SortedNumericDocValuesAnonymousInnerClass3>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SortedNumericDocValuesAnonymousInnerClass3>(
          SortedNumericDocValues::shared_from_this());
    }
  };

private:
  class SortedNumericDocValuesAnonymousInnerClass4
      : public SortedNumericDocValues
  {
    GET_CLASS_NAME(SortedNumericDocValuesAnonymousInnerClass4)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    std::shared_ptr<LongValues> ordinals;
    std::deque<int64_t> table;
    std::deque<int> offsets;

  public:
    SortedNumericDocValuesAnonymousInnerClass4(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        std::shared_ptr<LongValues> ordinals, std::deque<int64_t> &table,
        std::deque<int> &offsets);

    int startOffset = 0;
    int endOffset = 0;
    int docID = 0;
    int upto = 0;

    int docID() override;

    int nextDoc() override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int docValueCount() override;

    int64_t nextValue() override;

  protected:
    std::shared_ptr<SortedNumericDocValuesAnonymousInnerClass4>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SortedNumericDocValuesAnonymousInnerClass4>(
          SortedNumericDocValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<SortedSetDocValues>
  getSortedSet(std::shared_ptr<FieldInfo> field)  override;

private:
  std::shared_ptr<SortedSetDocValues> getSortedSetWithAddresses(
      std::shared_ptr<FieldInfo> field) ;

private:
  class LegacySortedSetDocValuesAnonymousInnerClass
      : public LegacySortedSetDocValues
  {
    GET_CLASS_NAME(LegacySortedSetDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    int64_t valueCount = 0;
    std::shared_ptr<org::apache::lucene::codecs::lucene54::
                        Lucene54DocValuesProducer::LongBinaryDocValues>
        binary;
    std::shared_ptr<LongValues> ordinals;
    std::shared_ptr<LongValues> ordIndex;

  public:
    LegacySortedSetDocValuesAnonymousInnerClass(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        int64_t valueCount,
        std::shared_ptr<org::apache::lucene::codecs::lucene54::
                            Lucene54DocValuesProducer::LongBinaryDocValues>
            binary,
        std::shared_ptr<LongValues> ordinals,
        std::shared_ptr<LongValues> ordIndex);

    int64_t startOffset = 0;
    int64_t offset = 0;
    int64_t endOffset = 0;

    int64_t nextOrd() override;

    void setDocument(int docID) override;

    std::shared_ptr<BytesRef> lookupOrd(int64_t ord) override;

    int64_t getValueCount() override;

    int64_t lookupTerm(std::shared_ptr<BytesRef> key) override;

    std::shared_ptr<TermsEnum> termsEnum()  override;

  protected:
    std::shared_ptr<LegacySortedSetDocValuesAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          LegacySortedSetDocValuesAnonymousInnerClass>(
          LegacySortedSetDocValues::shared_from_this());
    }
  };

private:
  std::shared_ptr<SortedSetDocValues>
  getSortedSetTable(std::shared_ptr<FieldInfo> field,
                    std::shared_ptr<SortedSetEntry> ss) ;

private:
  class LegacySortedSetDocValuesAnonymousInnerClass2
      : public LegacySortedSetDocValues
  {
    GET_CLASS_NAME(LegacySortedSetDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    int64_t valueCount = 0;
    std::shared_ptr<org::apache::lucene::codecs::lucene54::
                        Lucene54DocValuesProducer::LongBinaryDocValues>
        binary;
    std::shared_ptr<LongValues> ordinals;
    std::deque<int64_t> table;
    std::deque<int> offsets;

  public:
    LegacySortedSetDocValuesAnonymousInnerClass2(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance,
        int64_t valueCount,
        std::shared_ptr<org::apache::lucene::codecs::lucene54::
                            Lucene54DocValuesProducer::LongBinaryDocValues>
            binary,
        std::shared_ptr<LongValues> ordinals, std::deque<int64_t> &table,
        std::deque<int> &offsets);

    int offset = 0, startOffset = 0, endOffset = 0;

    void setDocument(int docID) override;

    int64_t nextOrd() override;

    std::shared_ptr<BytesRef> lookupOrd(int64_t ord) override;

    int64_t getValueCount() override;

    int64_t lookupTerm(std::shared_ptr<BytesRef> key) override;

    std::shared_ptr<TermsEnum> termsEnum()  override;

  protected:
    std::shared_ptr<LegacySortedSetDocValuesAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          LegacySortedSetDocValuesAnonymousInnerClass2>(
          LegacySortedSetDocValues::shared_from_this());
    }
  };

private:
  std::shared_ptr<Bits> getLiveBits(int64_t const offset,
                                    int const count) ;

private:
  class BitsAnonymousInnerClass
      : public std::enable_shared_from_this<BitsAnonymousInnerClass>,
        public Bits
  {
    GET_CLASS_NAME(BitsAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene54DocValuesProducer> outerInstance;

    int count = 0;
    std::shared_ptr<RandomAccessInput> in_;

  public:
    BitsAnonymousInnerClass(
        std::shared_ptr<Lucene54DocValuesProducer> outerInstance, int count,
        std::shared_ptr<RandomAccessInput> in_);

    bool get(int index) override;

    int length() override;
  };

private:
  std::shared_ptr<SparseNumericDocValues> getSparseNumericDocValues(
      std::shared_ptr<NumericEntry> entry) ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<DocValuesProducer>
  getMergeInstance()  override;

  virtual ~Lucene54DocValuesProducer();

  /** metadata entry for a numeric docvalues field */
public:
  class NumericEntry : public std::enable_shared_from_this<NumericEntry>
  {
    GET_CLASS_NAME(NumericEntry)
  private:
    NumericEntry();
    /** offset to the bitset representing docsWithField, or -1 if no documents
     * have missing values */
  public:
    int64_t missingOffset = 0;
    /** offset to the actual numeric values */
    int64_t offset = 0;
    /** end offset to the actual numeric values */
    int64_t endOffset = 0;
    /** bits per value used to pack the numeric values */
    int bitsPerValue = 0;

    int format = 0;
    /** count of values written */
    int64_t count = 0;

    /** monotonic meta */
    std::shared_ptr<DirectMonotonicReader::Meta> monotonicMeta;

    int64_t minValue = 0;
    int64_t gcd = 0;
    std::deque<int64_t> table;

    /** for sparse compression */
    int64_t numDocsWithValue = 0;
    std::shared_ptr<NumericEntry> nonMissingValues;
    NumberType numberType = static_cast<NumberType>(0);
  };

  /** metadata entry for a binary docvalues field */
public:
  class BinaryEntry : public std::enable_shared_from_this<BinaryEntry>
  {
    GET_CLASS_NAME(BinaryEntry)
  private:
    BinaryEntry();
    /** offset to the bitset representing docsWithField, or -1 if no documents
     * have missing values */
  public:
    int64_t missingOffset = 0;
    /** offset to the actual binary values */
    int64_t offset = 0;

    int format = 0;
    /** count of values written */
    int64_t count = 0;
    int minLength = 0;
    int maxLength = 0;
    /** offset to the addressing data that maps a value to its slice of the
     * byte[] */
    int64_t addressesOffset = 0, addressesEndOffset = 0;
    /** meta data for addresses */
    std::shared_ptr<DirectMonotonicReader::Meta> addressesMeta;
    /** offset to the reverse index */
    int64_t reverseIndexOffset = 0;
    /** packed ints version used to encode addressing information */
    int packedIntsVersion = 0;
    /** packed ints blocksize */
    int blockSize = 0;
  };

  /** metadata entry for a sorted-set docvalues field */
public:
  class SortedSetEntry : public std::enable_shared_from_this<SortedSetEntry>
  {
    GET_CLASS_NAME(SortedSetEntry)
  private:
    SortedSetEntry();

  public:
    int format = 0;

    std::deque<int64_t> table;
    std::deque<int> tableOffsets;
  };

  // internally we compose complex dv (sorted/sortedset) from other ones
public:
  class LongBinaryDocValues : public LegacyBinaryDocValues
  {
    GET_CLASS_NAME(LongBinaryDocValues)
  public:
    std::shared_ptr<BytesRef> get(int docID) override final;

    virtual std::shared_ptr<BytesRef> get(int64_t id) = 0;

  protected:
    std::shared_ptr<LongBinaryDocValues> shared_from_this()
    {
      return std::static_pointer_cast<LongBinaryDocValues>(
          LegacyBinaryDocValues::shared_from_this());
    }
  };

  // used for reverse lookup to a small range of blocks
public:
  class ReverseTermsIndex
      : public std::enable_shared_from_this<ReverseTermsIndex>,
        public Accountable
  {
    GET_CLASS_NAME(ReverseTermsIndex)
  public:
    std::shared_ptr<MonotonicBlockPackedReader> termAddresses;
    std::shared_ptr<PagedBytes::Reader> terms;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    virtual std::wstring toString();
  };

  // in the compressed case, we add a few additional operations for
  // more efficient reverse lookup and enumeration
public:
  class CompressedBinaryDocValues final : public LongBinaryDocValues
  {
    GET_CLASS_NAME(CompressedBinaryDocValues)
  public:
    const int64_t numValues;
    const int64_t numIndexValues;
    const int maxTermLength;
    const std::shared_ptr<MonotonicBlockPackedReader> addresses;
    const std::shared_ptr<IndexInput> data;
    const std::shared_ptr<CompressedBinaryTermsEnum> termsEnum;
    const std::shared_ptr<PagedBytes::Reader> reverseTerms;
    const std::shared_ptr<MonotonicBlockPackedReader> reverseAddresses;
    const int64_t numReverseIndexValues;

    CompressedBinaryDocValues(
        std::shared_ptr<BinaryEntry> bytes,
        std::shared_ptr<MonotonicBlockPackedReader> addresses,
        std::shared_ptr<ReverseTermsIndex> index,
        std::shared_ptr<IndexInput> data) ;

    std::shared_ptr<BytesRef> get(int64_t id) override;

    int64_t lookupTerm(std::shared_ptr<BytesRef> key);

    std::shared_ptr<TermsEnum> getTermsEnum() ;

  private:
    std::shared_ptr<CompressedBinaryTermsEnum>
    getTermsEnum(std::shared_ptr<IndexInput> input) ;

  public:
    class CompressedBinaryTermsEnum : public TermsEnum
    {
      GET_CLASS_NAME(CompressedBinaryTermsEnum)
    private:
      std::shared_ptr<Lucene54DocValuesProducer::CompressedBinaryDocValues>
          outerInstance;

      int64_t currentOrd = -1;
      // offset to the start of the current block
      int64_t currentBlockStart = 0;
      const std::shared_ptr<IndexInput> input;
      // delta from currentBlockStart to start of each term
      std::deque<int> const offsets =
          std::deque<int>(Lucene54DocValuesFormat::INTERVAL_COUNT);
      std::deque<char> const buffer =
          std::deque<char>(2 * Lucene54DocValuesFormat::INTERVAL_COUNT - 1);

      // C++ NOTE: Fields cannot have the same name as methods:
      const std::shared_ptr<BytesRef> term_ =
          std::make_shared<BytesRef>(outerInstance->maxTermLength);
      const std::shared_ptr<BytesRef> firstTerm =
          std::make_shared<BytesRef>(outerInstance->maxTermLength);
      const std::shared_ptr<BytesRef> scratch = std::make_shared<BytesRef>();

    public:
      CompressedBinaryTermsEnum(
          std::shared_ptr<Lucene54DocValuesProducer::CompressedBinaryDocValues>
              outerInstance,
          std::shared_ptr<IndexInput> input) ;

    private:
      void readHeader() ;

      // read single byte addresses: each is delta - 2
      // (shared prefix byte and length > 0 are both implicit)
      void readByteAddresses() ;

      // read double byte addresses: each is delta - 2
      // (shared prefix byte and length > 0 are both implicit)
      void readShortAddresses() ;

      // set term to the first term
      void readFirstTerm() ;

      // read term at offset, delta encoded from first term
      void readTerm(int offset) ;

    public:
      std::shared_ptr<BytesRef> next()  override;

      // binary search reverse index to find smaller
      // range of blocks to search
      virtual int64_t
      binarySearchIndex(std::shared_ptr<BytesRef> text) ;

      // binary search against first term in block range
      // to find term's block
      virtual int64_t binarySearchBlock(std::shared_ptr<BytesRef> text,
                                          int64_t low,
                                          int64_t high) ;

      SeekStatus
      seekCeil(std::shared_ptr<BytesRef> text)  override;

      void seekExact(int64_t ord)  override;

      std::shared_ptr<BytesRef> term()  override;

      int64_t ord()  override;

      int docFreq()  override;

      int64_t totalTermFreq()  override;

      std::shared_ptr<PostingsEnum>
      postings(std::shared_ptr<PostingsEnum> reuse,
               int flags)  override;

    protected:
      std::shared_ptr<CompressedBinaryTermsEnum> shared_from_this()
      {
        return std::static_pointer_cast<CompressedBinaryTermsEnum>(
            TermsEnum::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<CompressedBinaryDocValues> shared_from_this()
    {
      return std::static_pointer_cast<CompressedBinaryDocValues>(
          LongBinaryDocValues::shared_from_this());
    }
  };

protected:
  std::shared_ptr<Lucene54DocValuesProducer> shared_from_this()
  {
    return std::static_pointer_cast<Lucene54DocValuesProducer>(
        org.apache.lucene.codecs.DocValuesProducer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene54/
