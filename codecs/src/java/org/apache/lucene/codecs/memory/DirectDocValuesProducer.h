#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/memory/NumericEntry.h"

#include  "core/src/java/org/apache/lucene/codecs/memory/BinaryEntry.h"
#include  "core/src/java/org/apache/lucene/codecs/memory/SortedEntry.h"
#include  "core/src/java/org/apache/lucene/codecs/memory/SortedSetEntry.h"
#include  "core/src/java/org/apache/lucene/codecs/memory/SortedNumericEntry.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/codecs/memory/NumericRawValues.h"
#include  "core/src/java/org/apache/lucene/codecs/memory/BinaryRawValues.h"
#include  "core/src/java/org/apache/lucene/codecs/memory/SortedRawValues.h"
#include  "core/src/java/org/apache/lucene/codecs/memory/SortedSetRawValues.h"
#include  "core/src/java/org/apache/lucene/codecs/memory/SortedNumericRawValues.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/LegacyNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/LegacyBinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"

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
namespace org::apache::lucene::codecs::memory
{

using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using namespace org::apache::lucene::index;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

/**
 * Reader for {@link DirectDocValuesFormat}
 */

class DirectDocValuesProducer : public DocValuesProducer
{
  GET_CLASS_NAME(DirectDocValuesProducer)
  // metadata maps (just file pointers and minimal stuff)
private:
  const std::unordered_map<std::wstring, std::shared_ptr<NumericEntry>>
      numerics =
          std::unordered_map<std::wstring, std::shared_ptr<NumericEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<BinaryEntry>>
      binaries =
          std::unordered_map<std::wstring, std::shared_ptr<BinaryEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<SortedEntry>> sorteds =
      std::unordered_map<std::wstring, std::shared_ptr<SortedEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<SortedSetEntry>>
      sortedSets =
          std::unordered_map<std::wstring, std::shared_ptr<SortedSetEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<SortedNumericEntry>>
      sortedNumerics =
          std::unordered_map<std::wstring,
                             std::shared_ptr<SortedNumericEntry>>();
  const std::shared_ptr<IndexInput> data;

  // ram instances we have already loaded
  const std::unordered_map<std::wstring, std::shared_ptr<NumericRawValues>>
      numericInstances =
          std::unordered_map<std::wstring, std::shared_ptr<NumericRawValues>>();
  const std::unordered_map<std::wstring, std::shared_ptr<BinaryRawValues>>
      binaryInstances =
          std::unordered_map<std::wstring, std::shared_ptr<BinaryRawValues>>();
  const std::unordered_map<std::wstring, std::shared_ptr<SortedRawValues>>
      sortedInstances =
          std::unordered_map<std::wstring, std::shared_ptr<SortedRawValues>>();
  const std::unordered_map<std::wstring, std::shared_ptr<SortedSetRawValues>>
      sortedSetInstances =
          std::unordered_map<std::wstring,
                             std::shared_ptr<SortedSetRawValues>>();
  const std::unordered_map<std::wstring,
                           std::shared_ptr<SortedNumericRawValues>>
      sortedNumericInstances =
          std::unordered_map<std::wstring,
                             std::shared_ptr<SortedNumericRawValues>>();
  const std::unordered_map<std::wstring, std::shared_ptr<FixedBitSet>>
      docsWithFieldInstances =
          std::unordered_map<std::wstring, std::shared_ptr<FixedBitSet>>();

  const int numEntries;

  const int maxDoc;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<AtomicLong> ramBytesUsed_;
  const int version;

  const bool merging;

public:
  static constexpr char NUMBER = 0;
  static constexpr char BYTES = 1;
  static constexpr char SORTED = 2;
  static constexpr char SORTED_SET = 3;
  static constexpr char SORTED_SET_SINGLETON = 4;
  static constexpr char SORTED_NUMERIC = 5;
  static constexpr char SORTED_NUMERIC_SINGLETON = 6;

  static constexpr int VERSION_START = 3;
  static constexpr int VERSION_CURRENT = VERSION_START;

  // clone for merge: when merging we don't do any instances.put()s
  DirectDocValuesProducer(
      std::shared_ptr<DirectDocValuesProducer> original) ;

  DirectDocValuesProducer(std::shared_ptr<SegmentReadState> state,
                          const std::wstring &dataCodec,
                          const std::wstring &dataExtension,
                          const std::wstring &metaCodec,
                          const std::wstring &metaExtension) ;

private:
  std::shared_ptr<NumericEntry>
  readNumericEntry(std::shared_ptr<IndexInput> meta) ;

  std::shared_ptr<BinaryEntry>
  readBinaryEntry(std::shared_ptr<IndexInput> meta) ;

  std::shared_ptr<SortedEntry>
  readSortedEntry(std::shared_ptr<IndexInput> meta) ;

  std::shared_ptr<SortedSetEntry>
  readSortedSetEntry(std::shared_ptr<IndexInput> meta,
                     bool singleton) ;

  std::shared_ptr<SortedNumericEntry>
  readSortedNumericEntry(std::shared_ptr<IndexInput> meta,
                         bool singleton) ;

  int readFields(std::shared_ptr<IndexInput> meta,
                 std::shared_ptr<FieldInfos> infos) ;

public:
  int64_t ramBytesUsed() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  virtual std::wstring toString();

  void checkIntegrity()  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<NumericDocValues>
  getNumeric(std::shared_ptr<FieldInfo> field)  override;

private:
  std::shared_ptr<NumericRawValues>
  loadNumeric(std::shared_ptr<NumericEntry> entry) ;

private:
  class LegacyNumericDocValuesAnonymousInnerClass
      : public LegacyNumericDocValues
  {
    GET_CLASS_NAME(LegacyNumericDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<DirectDocValuesProducer> outerInstance;

    std::deque<char> values;

  public:
    LegacyNumericDocValuesAnonymousInnerClass(
        std::shared_ptr<DirectDocValuesProducer> outerInstance,
        std::deque<char> &values);

    int64_t get(int idx) override;

  protected:
    std::shared_ptr<LegacyNumericDocValuesAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          LegacyNumericDocValuesAnonymousInnerClass>(
          LegacyNumericDocValues::shared_from_this());
    }
  };

private:
  class LegacyNumericDocValuesAnonymousInnerClass2
      : public LegacyNumericDocValues
  {
    GET_CLASS_NAME(LegacyNumericDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<DirectDocValuesProducer> outerInstance;

    std::deque<short> values;

  public:
    LegacyNumericDocValuesAnonymousInnerClass2(
        std::shared_ptr<DirectDocValuesProducer> outerInstance,
        std::deque<short> &values);

    int64_t get(int idx) override;

  protected:
    std::shared_ptr<LegacyNumericDocValuesAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          LegacyNumericDocValuesAnonymousInnerClass2>(
          LegacyNumericDocValues::shared_from_this());
    }
  };

private:
  class LegacyNumericDocValuesAnonymousInnerClass3
      : public LegacyNumericDocValues
  {
    GET_CLASS_NAME(LegacyNumericDocValuesAnonymousInnerClass3)
  private:
    std::shared_ptr<DirectDocValuesProducer> outerInstance;

    std::deque<int> values;

  public:
    LegacyNumericDocValuesAnonymousInnerClass3(
        std::shared_ptr<DirectDocValuesProducer> outerInstance,
        std::deque<int> &values);

    int64_t get(int idx) override;

  protected:
    std::shared_ptr<LegacyNumericDocValuesAnonymousInnerClass3>
    shared_from_this()
    {
      return std::static_pointer_cast<
          LegacyNumericDocValuesAnonymousInnerClass3>(
          LegacyNumericDocValues::shared_from_this());
    }
  };

private:
  class LegacyNumericDocValuesAnonymousInnerClass4
      : public LegacyNumericDocValues
  {
    GET_CLASS_NAME(LegacyNumericDocValuesAnonymousInnerClass4)
  private:
    std::shared_ptr<DirectDocValuesProducer> outerInstance;

    std::deque<int64_t> values;

  public:
    LegacyNumericDocValuesAnonymousInnerClass4(
        std::shared_ptr<DirectDocValuesProducer> outerInstance,
        std::deque<int64_t> &values);

    int64_t get(int idx) override;

  protected:
    std::shared_ptr<LegacyNumericDocValuesAnonymousInnerClass4>
    shared_from_this()
    {
      return std::static_pointer_cast<
          LegacyNumericDocValuesAnonymousInnerClass4>(
          LegacyNumericDocValues::shared_from_this());
    }
  };

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<LegacyBinaryDocValues>
  getLegacyBinary(std::shared_ptr<FieldInfo> field) ;

private:
  class LegacyBinaryDocValuesAnonymousInnerClass : public LegacyBinaryDocValues
  {
    GET_CLASS_NAME(LegacyBinaryDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<DirectDocValuesProducer> outerInstance;

    std::deque<char> bytes;
    std::deque<int> address;

  public:
    LegacyBinaryDocValuesAnonymousInnerClass(
        std::shared_ptr<DirectDocValuesProducer> outerInstance,
        std::deque<char> &bytes, std::deque<int> &address);

    const std::shared_ptr<BytesRef> term;

    std::shared_ptr<BytesRef> get(int docID) override;

  protected:
    std::shared_ptr<LegacyBinaryDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LegacyBinaryDocValuesAnonymousInnerClass>(
          LegacyBinaryDocValues::shared_from_this());
    }
  };

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<BinaryDocValues>
  getBinary(std::shared_ptr<FieldInfo> field)  override;

private:
  std::shared_ptr<BinaryRawValues>
  loadBinary(std::shared_ptr<BinaryEntry> entry) ;

public:
  std::shared_ptr<SortedDocValues>
  getSorted(std::shared_ptr<FieldInfo> field)  override;

private:
  std::shared_ptr<LegacySortedDocValues>
  newSortedInstance(std::shared_ptr<LegacyNumericDocValues> docToOrd,
                    std::shared_ptr<LegacyBinaryDocValues> values,
                    int const count);

private:
  class LegacySortedDocValuesAnonymousInnerClass : public LegacySortedDocValues
  {
    GET_CLASS_NAME(LegacySortedDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<DirectDocValuesProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues>
        docToOrd;
    std::shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues> values;
    int count = 0;

  public:
    LegacySortedDocValuesAnonymousInnerClass(
        std::shared_ptr<DirectDocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues>
            docToOrd,
        std::shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues>
            values,
        int count);

    int getOrd(int docID) override;

    std::shared_ptr<BytesRef> lookupOrd(int ord) override;

    int getValueCount() override;

    // Leave lookupTerm to super's binary search

    // Leave termsEnum to super

  protected:
    std::shared_ptr<LegacySortedDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LegacySortedDocValuesAnonymousInnerClass>(
          LegacySortedDocValues::shared_from_this());
    }
  };

private:
  std::shared_ptr<SortedRawValues>
  loadSorted(std::shared_ptr<FieldInfo> field) ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<SortedNumericDocValues> getSortedNumeric(
      std::shared_ptr<FieldInfo> field)  override;

private:
  class LegacySortedNumericDocValuesAnonymousInnerClass
      : public LegacySortedNumericDocValues
  {
    GET_CLASS_NAME(LegacySortedNumericDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<DirectDocValuesProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues>
        docToAddress;
    std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues> values;

  public:
    LegacySortedNumericDocValuesAnonymousInnerClass(
        std::shared_ptr<DirectDocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues>
            docToAddress,
        std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues>
            values);

    int valueStart = 0;
    int valueLimit = 0;

    void setDocument(int doc) override;

    int64_t valueAt(int index) override;

    int count() override;

  protected:
    std::shared_ptr<LegacySortedNumericDocValuesAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          LegacySortedNumericDocValuesAnonymousInnerClass>(
          LegacySortedNumericDocValues::shared_from_this());
    }
  };

private:
  std::shared_ptr<SortedNumericRawValues> loadSortedNumeric(
      std::shared_ptr<SortedNumericEntry> entry) ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<SortedSetDocValues>
  getSortedSet(std::shared_ptr<FieldInfo> field)  override;

private:
  class LegacySortedSetDocValuesAnonymousInnerClass
      : public LegacySortedSetDocValues
  {
    GET_CLASS_NAME(LegacySortedSetDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<DirectDocValuesProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::codecs::memory::
                        DirectDocValuesProducer::SortedSetEntry>
        entry;
    std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues>
        docToOrdAddress;
    std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues> ords;
    std::shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues> values;

  public:
    LegacySortedSetDocValuesAnonymousInnerClass(
        std::shared_ptr<DirectDocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::memory::
                            DirectDocValuesProducer::SortedSetEntry>
            entry,
        std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues>
            docToOrdAddress,
        std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues>
            ords,
        std::shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues>
            values);

    int ordStart = 0;
    int ordUpto = 0;
    int ordLimit = 0;

    int64_t nextOrd() override;

    void setDocument(int docID) override;

    std::shared_ptr<BytesRef> lookupOrd(int64_t ord) override;

    int64_t getValueCount() override;

    // Leave lookupTerm to super's binary search

    // Leave termsEnum to super

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
  std::shared_ptr<SortedSetRawValues>
  loadSortedSet(std::shared_ptr<SortedSetEntry> entry) ;

  std::shared_ptr<Bits>
  getMissingBits(std::shared_ptr<FieldInfo> field, int64_t const offset,
                 int64_t const length) ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<DocValuesProducer>
  getMergeInstance()  override;

  virtual ~DirectDocValuesProducer();

public:
  class BinaryRawValues : public std::enable_shared_from_this<BinaryRawValues>,
                          public Accountable
  {
    GET_CLASS_NAME(BinaryRawValues)
  public:
    std::deque<char> bytes;
    std::deque<int> address;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    virtual std::wstring toString();
  };

public:
  class NumericRawValues
      : public std::enable_shared_from_this<NumericRawValues>,
        public Accountable
  {
    GET_CLASS_NAME(NumericRawValues)
  public:
    std::shared_ptr<LegacyNumericDocValues> numerics;
    int64_t bytesUsed = 0;

    int64_t ramBytesUsed() override;

    virtual std::wstring toString();
  };

public:
  class SortedRawValues : public std::enable_shared_from_this<SortedRawValues>,
                          public Accountable
  {
    GET_CLASS_NAME(SortedRawValues)
  public:
    std::shared_ptr<NumericRawValues> docToOrd;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    virtual std::wstring toString();
  };

public:
  class SortedNumericRawValues
      : public std::enable_shared_from_this<SortedNumericRawValues>,
        public Accountable
  {
    GET_CLASS_NAME(SortedNumericRawValues)
  public:
    std::shared_ptr<NumericRawValues> docToAddress;
    std::shared_ptr<NumericRawValues> values;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    virtual std::wstring toString();
  };

public:
  class SortedSetRawValues
      : public std::enable_shared_from_this<SortedSetRawValues>,
        public Accountable
  {
    GET_CLASS_NAME(SortedSetRawValues)
  public:
    std::shared_ptr<NumericRawValues> docToOrdAddress;
    std::shared_ptr<NumericRawValues> ords;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    virtual std::wstring toString();
  };

public:
  class NumericEntry : public std::enable_shared_from_this<NumericEntry>
  {
    GET_CLASS_NAME(NumericEntry)
  public:
    int64_t offset = 0;
    int count = 0;
    int64_t missingOffset = 0;
    int64_t missingBytes = 0;
    char byteWidth = 0;
    int packedIntsVersion = 0;
  };

public:
  class BinaryEntry : public std::enable_shared_from_this<BinaryEntry>
  {
    GET_CLASS_NAME(BinaryEntry)
  public:
    int64_t offset = 0;
    int64_t missingOffset = 0;
    int64_t missingBytes = 0;
    int count = 0;
    int numBytes = 0;
    int minLength = 0;
    int maxLength = 0;
    int packedIntsVersion = 0;
    int blockSize = 0;
  };

public:
  class SortedEntry : public std::enable_shared_from_this<SortedEntry>
  {
    GET_CLASS_NAME(SortedEntry)
  public:
    std::shared_ptr<NumericEntry> docToOrd;
    std::shared_ptr<BinaryEntry> values;
  };

public:
  class SortedSetEntry : public std::enable_shared_from_this<SortedSetEntry>
  {
    GET_CLASS_NAME(SortedSetEntry)
  public:
    std::shared_ptr<NumericEntry> docToOrdAddress;
    std::shared_ptr<NumericEntry> ords;
    std::shared_ptr<BinaryEntry> values;
  };

public:
  class SortedNumericEntry
      : public std::enable_shared_from_this<SortedNumericEntry>
  {
    GET_CLASS_NAME(SortedNumericEntry)
  public:
    std::shared_ptr<NumericEntry> docToAddress;
    std::shared_ptr<NumericEntry> values;
  };

public:
  class FSTEntry : public std::enable_shared_from_this<FSTEntry>
  {
    GET_CLASS_NAME(FSTEntry)
  public:
    int64_t offset = 0;
    int64_t numOrds = 0;
  };

protected:
  std::shared_ptr<DirectDocValuesProducer> shared_from_this()
  {
    return std::static_pointer_cast<DirectDocValuesProducer>(
        org.apache.lucene.codecs.DocValuesProducer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/memory/
