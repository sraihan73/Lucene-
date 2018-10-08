#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/BytesRefFSTEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::memory
{
class NumericEntry;
}

namespace org::apache::lucene::codecs::memory
{
class BinaryEntry;
}
namespace org::apache::lucene::codecs::memory
{
class FSTEntry;
}
namespace org::apache::lucene::codecs::memory
{
class SortedSetEntry;
}
namespace org::apache::lucene::codecs::memory
{
class SortedNumericEntry;
}
namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::codecs::memory
{
class BytesAndAddresses;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::util::packed
{
class MonotonicBlockPackedReader;
}
namespace org::apache::lucene::util
{
class Accountable;
}
namespace org::apache::lucene::codecs
{
class DocValuesProducer;
}
namespace org::apache::lucene::util::packed
{
class PackedInts;
}
namespace org::apache::lucene::util::packed
{
class Reader;
}
namespace org::apache::lucene::util::packed
{
class BlockPackedReader;
}
namespace org::apache::lucene::util
{
class PagedBytes;
}
namespace org::apache::lucene::util
{
class Reader;
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
class LegacyNumericDocValues;
}
namespace org::apache::lucene::util::fst
{
class BytesReader;
}
namespace org::apache::lucene::util
{
class IntsRefBuilder;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class BytesRefFSTEnum;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class Arc;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::index
{
class LegacyBinaryDocValues;
}
namespace org::apache::lucene::store
{
class ByteArrayDataInput;
}
namespace org::apache::lucene::util
{
class Bits;
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
namespace org::apache::lucene::codecs::memory
{

using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using namespace org::apache::lucene::index;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using PagedBytes = org::apache::lucene::util::PagedBytes;
using BytesRefFSTEnum = org::apache::lucene::util::fst::BytesRefFSTEnum;
using org::apache::lucene::util::fst::FST::Arc;
using BytesReader = org::apache::lucene::util::fst::FST::BytesReader;
using FST = org::apache::lucene::util::fst::FST;
using MonotonicBlockPackedReader =
    org::apache::lucene::util::packed::MonotonicBlockPackedReader;

/**
 * Reader for {@link MemoryDocValuesFormat}
 */
class MemoryDocValuesProducer
    : public org.apache::lucene::codecs::DocValuesProducer
{
  GET_CLASS_NAME(MemoryDocValuesProducer)
  // metadata maps (just file pointers and minimal stuff)
private:
  const std::unordered_map<std::wstring, std::shared_ptr<NumericEntry>>
      numerics =
          std::unordered_map<std::wstring, std::shared_ptr<NumericEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<BinaryEntry>>
      binaries =
          std::unordered_map<std::wstring, std::shared_ptr<BinaryEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<FSTEntry>> fsts =
      std::unordered_map<std::wstring, std::shared_ptr<FSTEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<SortedSetEntry>>
      sortedSets =
          std::unordered_map<std::wstring, std::shared_ptr<SortedSetEntry>>();
  const std::unordered_map<std::wstring, std::shared_ptr<SortedNumericEntry>>
      sortedNumerics =
          std::unordered_map<std::wstring,
                             std::shared_ptr<SortedNumericEntry>>();
  const std::shared_ptr<IndexInput> data;

  // ram instances we have already loaded
  const std::unordered_map<std::wstring,
                           std::shared_ptr<LegacyNumericDocValues>>
      numericInstances =
          std::unordered_map<std::wstring,
                             std::shared_ptr<LegacyNumericDocValues>>();
  const std::unordered_map<std::wstring, std::shared_ptr<BytesAndAddresses>>
      pagedBytesInstances =
          std::unordered_map<std::wstring,
                             std::shared_ptr<BytesAndAddresses>>();
  const std::unordered_map<std::wstring, FST<int64_t>> fstInstances =
      std::unordered_map<std::wstring, FST<int64_t>>();
  const std::unordered_map<std::wstring, std::shared_ptr<FixedBitSet>>
      docsWithFieldInstances =
          std::unordered_map<std::wstring, std::shared_ptr<FixedBitSet>>();
  const std::unordered_map<std::wstring,
                           std::shared_ptr<MonotonicBlockPackedReader>>
      addresses =
          std::unordered_map<std::wstring,
                             std::shared_ptr<MonotonicBlockPackedReader>>();

  const std::unordered_map<std::wstring, std::shared_ptr<Accountable>>
      numericInfo =
          std::unordered_map<std::wstring, std::shared_ptr<Accountable>>();

  const int numEntries;
  const int maxDoc;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<AtomicLong> ramBytesUsed_;
  const int version;

  const bool merging;

public:
  static constexpr char NUMBER = 0;
  static constexpr char BYTES = 1;
  static constexpr char org = 0;
  static constexpr char SORTED_SET = 4;
  static constexpr char SORTED_SET_SINGLETON = 5;
  static constexpr char SORTED_NUMERIC = 6;
  static constexpr char SORTED_NUMERIC_SINGLETON = 7;

  static constexpr int BLOCK_SIZE = 4096;

  static constexpr char DELTA_COMPRESSED = 0;
  static constexpr char TABLE_COMPRESSED = 1;
  static constexpr char BLOCK_COMPRESSED = 2;
  static constexpr char GCD_COMPRESSED = 3;

  static constexpr int VERSION_START = 4;
  static constexpr int VERSION_CURRENT = VERSION_START;

  // clone for merge: when merging we don't do any instances.put()s
  MemoryDocValuesProducer(
      std::shared_ptr<MemoryDocValuesProducer> original) ;

  MemoryDocValuesProducer(std::shared_ptr<SegmentReadState> state,
                          const std::wstring &dataCodec,
                          const std::wstring &dataExtension,
                          const std::wstring &metaCodec,
                          const std::wstring &metaExtension) ;

private:
  std::shared_ptr<NumericEntry>
  readNumericEntry(std::shared_ptr<IndexInput> meta) ;

  std::shared_ptr<BinaryEntry>
  readBinaryEntry(std::shared_ptr<IndexInput> meta) ;

  std::shared_ptr<FSTEntry>
  readFSTEntry(std::shared_ptr<IndexInput> meta) ;

  int readFields(std::shared_ptr<IndexInput> meta,
                 std::shared_ptr<FieldInfos> infos) ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<NumericDocValues>
  getNumeric(std::shared_ptr<FieldInfo> field)  override;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<LegacyNumericDocValues>
  getNumericNonIterator(std::shared_ptr<FieldInfo> field) ;

public:
  int64_t ramBytesUsed() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  void checkIntegrity()  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<DocValuesProducer>
  getMergeInstance()  override;

  virtual std::wstring toString();

private:
  std::shared_ptr<LegacyNumericDocValues>
  loadNumeric(std::shared_ptr<FieldInfo> field) ;

private:
  class LegacyNumericDocValuesAnonymousInnerClass
      : public LegacyNumericDocValues
  {
    GET_CLASS_NAME(LegacyNumericDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<MemoryDocValuesProducer> outerInstance;

    std::deque<int64_t> decode;
    std::shared_ptr<PackedInts::Reader> ordsReader;

  public:
    LegacyNumericDocValuesAnonymousInnerClass(
        std::shared_ptr<MemoryDocValuesProducer> outerInstance,
        std::deque<int64_t> &decode,
        std::shared_ptr<PackedInts::Reader> ordsReader);

    int64_t get(int docID) override;

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
    std::shared_ptr<MemoryDocValuesProducer> outerInstance;

    int64_t minDelta = 0;
    std::shared_ptr<PackedInts::Reader> deltaReader;

  public:
    LegacyNumericDocValuesAnonymousInnerClass2(
        std::shared_ptr<MemoryDocValuesProducer> outerInstance,
        int64_t minDelta, std::shared_ptr<PackedInts::Reader> deltaReader);

    int64_t get(int docID) override;

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
    std::shared_ptr<MemoryDocValuesProducer> outerInstance;

    std::shared_ptr<BlockPackedReader> reader;

  public:
    LegacyNumericDocValuesAnonymousInnerClass3(
        std::shared_ptr<MemoryDocValuesProducer> outerInstance,
        std::shared_ptr<BlockPackedReader> reader);

    int64_t get(int docID) override;

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
    std::shared_ptr<MemoryDocValuesProducer> outerInstance;

    int64_t min = 0;
    int64_t mult = 0;
    std::shared_ptr<PackedInts::Reader> quotientReader;

  public:
    LegacyNumericDocValuesAnonymousInnerClass4(
        std::shared_ptr<MemoryDocValuesProducer> outerInstance, int64_t min,
        int64_t mult, std::shared_ptr<PackedInts::Reader> quotientReader);

    int64_t get(int docID) override;

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
  std::shared_ptr<LegacyBinaryDocValues>
  getLegacyBinary(std::shared_ptr<FieldInfo> field) ;

private:
  class LegacyBinaryDocValuesAnonymousInnerClass : public LegacyBinaryDocValues
  {
    GET_CLASS_NAME(LegacyBinaryDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<MemoryDocValuesProducer> outerInstance;

    std::shared_ptr<PagedBytes::Reader> bytesReader;
    int fixedLength = 0;

  public:
    LegacyBinaryDocValuesAnonymousInnerClass(
        std::shared_ptr<MemoryDocValuesProducer> outerInstance,
        std::shared_ptr<PagedBytes::Reader> bytesReader, int fixedLength);

    const std::shared_ptr<BytesRef> term;

    std::shared_ptr<BytesRef> get(int docID) override;

  protected:
    std::shared_ptr<LegacyBinaryDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LegacyBinaryDocValuesAnonymousInnerClass>(
          LegacyBinaryDocValues::shared_from_this());
    }
  };

private:
  class LegacyBinaryDocValuesAnonymousInnerClass2 : public LegacyBinaryDocValues
  {
    GET_CLASS_NAME(LegacyBinaryDocValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<MemoryDocValuesProducer> outerInstance;

    std::shared_ptr<PagedBytes::Reader> bytesReader;
    std::shared_ptr<MonotonicBlockPackedReader> addresses;

  public:
    LegacyBinaryDocValuesAnonymousInnerClass2(
        std::shared_ptr<MemoryDocValuesProducer> outerInstance,
        std::shared_ptr<PagedBytes::Reader> bytesReader,
        std::shared_ptr<MonotonicBlockPackedReader> addresses);

    const std::shared_ptr<BytesRef> term;

    std::shared_ptr<BytesRef> get(int docID) override;

  protected:
    std::shared_ptr<LegacyBinaryDocValuesAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          LegacyBinaryDocValuesAnonymousInnerClass2>(
          LegacyBinaryDocValues::shared_from_this());
    }
  };

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<BinaryDocValues>
  getBinary(std::shared_ptr<FieldInfo> field)  override;

private:
  std::shared_ptr<BytesAndAddresses>
  loadBinary(std::shared_ptr<FieldInfo> field) ;

public:
  std::shared_ptr<SortedDocValues>
  getSorted(std::shared_ptr<FieldInfo> field)  override;

private:
  std::shared_ptr<LegacySortedDocValues>
  getSortedNonIterator(std::shared_ptr<FieldInfo> field) ;

private:
  class LegacySortedDocValuesAnonymousInnerClass : public LegacySortedDocValues
  {
    GET_CLASS_NAME(LegacySortedDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<MemoryDocValuesProducer> outerInstance;

    std::shared_ptr<
        org::apache::lucene::codecs::memory::MemoryDocValuesProducer::FSTEntry>
        entry;
    std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues>
        docToOrd;
    std::shared_ptr<FST<int64_t>> fst;
    std::shared_ptr<BytesReader> in_;
    std::shared_ptr<Arc<int64_t>> firstArc;
    std::shared_ptr<Arc<int64_t>> scratchArc;
    std::shared_ptr<IntsRefBuilder> scratchInts;
    std::shared_ptr<BytesRefFSTEnum<int64_t>> fstEnum;

  public:
    LegacySortedDocValuesAnonymousInnerClass(
        std::shared_ptr<MemoryDocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::memory::
                            MemoryDocValuesProducer::FSTEntry>
            entry,
        std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues>
            docToOrd,
        std::shared_ptr<FST<int64_t>> fst, std::shared_ptr<BytesReader> in_,
        std::shared_ptr<Arc<int64_t>> firstArc,
        std::shared_ptr<Arc<int64_t>> scratchArc,
        std::shared_ptr<IntsRefBuilder> scratchInts,
        std::shared_ptr<BytesRefFSTEnum<int64_t>> fstEnum);

    const std::shared_ptr<BytesRefBuilder> term;

    int getOrd(int docID) override;

    std::shared_ptr<BytesRef> lookupOrd(int ord) override;

    int lookupTerm(std::shared_ptr<BytesRef> key) override;

    int getValueCount() override;

    std::shared_ptr<TermsEnum> termsEnum() override;

  protected:
    std::shared_ptr<LegacySortedDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LegacySortedDocValuesAnonymousInnerClass>(
          LegacySortedDocValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<SortedNumericDocValues> getSortedNumeric(
      std::shared_ptr<FieldInfo> field)  override;

private:
  class LegacySortedNumericDocValuesAnonymousInnerClass
      : public LegacySortedNumericDocValues
  {
    GET_CLASS_NAME(LegacySortedNumericDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<MemoryDocValuesProducer> outerInstance;

    std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues> values;
    std::shared_ptr<MonotonicBlockPackedReader> addr;

  public:
    LegacySortedNumericDocValuesAnonymousInnerClass(
        std::shared_ptr<MemoryDocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::index::LegacyNumericDocValues>
            values,
        std::shared_ptr<MonotonicBlockPackedReader> addr);

    int startOffset = 0;
    int endOffset = 0;

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

public:
  std::shared_ptr<SortedSetDocValues>
  getSortedSet(std::shared_ptr<FieldInfo> field)  override;

private:
  class LegacySortedSetDocValuesAnonymousInnerClass
      : public LegacySortedSetDocValues
  {
    GET_CLASS_NAME(LegacySortedSetDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<MemoryDocValuesProducer> outerInstance;

    std::shared_ptr<
        org::apache::lucene::codecs::memory::MemoryDocValuesProducer::FSTEntry>
        entry;
    std::shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues>
        docToOrds;
    std::shared_ptr<FST<int64_t>> fst;
    std::shared_ptr<BytesReader> in_;
    std::shared_ptr<Arc<int64_t>> firstArc;
    std::shared_ptr<Arc<int64_t>> scratchArc;
    std::shared_ptr<IntsRefBuilder> scratchInts;
    std::shared_ptr<BytesRefFSTEnum<int64_t>> fstEnum;
    std::shared_ptr<ByteArrayDataInput> input;

  public:
    LegacySortedSetDocValuesAnonymousInnerClass(
        std::shared_ptr<MemoryDocValuesProducer> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::memory::
                            MemoryDocValuesProducer::FSTEntry>
            entry,
        std::shared_ptr<org::apache::lucene::index::LegacyBinaryDocValues>
            docToOrds,
        std::shared_ptr<FST<int64_t>> fst, std::shared_ptr<BytesReader> in_,
        std::shared_ptr<Arc<int64_t>> firstArc,
        std::shared_ptr<Arc<int64_t>> scratchArc,
        std::shared_ptr<IntsRefBuilder> scratchInts,
        std::shared_ptr<BytesRefFSTEnum<int64_t>> fstEnum,
        std::shared_ptr<ByteArrayDataInput> input);

    const std::shared_ptr<BytesRefBuilder> term;
    std::shared_ptr<BytesRef> ref;
    int64_t currentOrd = 0;

    int64_t nextOrd() override;

    void setDocument(int docID) override;

    std::shared_ptr<BytesRef> lookupOrd(int64_t ord) override;

    int64_t lookupTerm(std::shared_ptr<BytesRef> key) override;

    int64_t getValueCount() override;

    std::shared_ptr<TermsEnum> termsEnum() override;

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
  std::shared_ptr<Bits>
  getMissingBits(std::shared_ptr<FieldInfo> field, int64_t const offset,
                 int64_t const length) ;

public:
  virtual ~MemoryDocValuesProducer();

public:
  class NumericEntry : public std::enable_shared_from_this<NumericEntry>
  {
    GET_CLASS_NAME(NumericEntry)
  public:
    int64_t offset = 0;
    int64_t count = 0;
    int64_t missingOffset = 0;
    int64_t missingBytes = 0;
    char format = 0;
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
    int64_t numBytes = 0;
    int minLength = 0;
    int maxLength = 0;
    int packedIntsVersion = 0;
    int blockSize = 0;
  };

public:
  class FSTEntry : public std::enable_shared_from_this<FSTEntry>
  {
    GET_CLASS_NAME(FSTEntry)
  public:
    int64_t offset = 0;
    int64_t numOrds = 0;
  };

public:
  class SortedSetEntry : public std::enable_shared_from_this<SortedSetEntry>
  {
    GET_CLASS_NAME(SortedSetEntry)
  public:
    bool singleton = false;
  };

public:
  class SortedNumericEntry
      : public std::enable_shared_from_this<SortedNumericEntry>
  {
    GET_CLASS_NAME(SortedNumericEntry)
  public:
    bool singleton = false;
    int64_t addressOffset = 0;
    int packedIntsVersion = 0;
    int blockSize = 0;
    int64_t valueCount = 0;
  };

public:
  class BytesAndAddresses
      : public std::enable_shared_from_this<BytesAndAddresses>,
        public org.apache::lucene::util::Accountable
  {
    GET_CLASS_NAME(BytesAndAddresses)
  public:
    std::shared_ptr<PagedBytes::Reader> reader;
    std::shared_ptr<MonotonicBlockPackedReader> addresses;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;
  };

  // exposes FSTEnum directly as a TermsEnum: avoids binary-search next()
public:
  class FSTTermsEnum : public TermsEnum
  {
    GET_CLASS_NAME(FSTTermsEnum)
  public:
    const std::shared_ptr<BytesRefFSTEnum<int64_t>> in_;

    // this is all for the complicated seek(ord)...
    // maybe we should add a FSTEnum that supports this operation?
    const std::shared_ptr<FST<int64_t>> fst;
    const std::shared_ptr<BytesReader> bytesReader;
    const std::shared_ptr<Arc<int64_t>> firstArc =
        std::make_shared<org.apache::lucene::util::fst::FST::Arc<int64_t>>();
    const std::shared_ptr<Arc<int64_t>> scratchArc =
        std::make_shared<org.apache::lucene::util::fst::FST::Arc<int64_t>>();
    const std::shared_ptr<IntsRefBuilder> scratchInts =
        std::make_shared<org.apache::lucene::util::IntsRefBuilder>();
    const std::shared_ptr<BytesRefBuilder> scratchBytes =
        std::make_shared<org.apache::lucene::util::BytesRefBuilder>();

    FSTTermsEnum(std::shared_ptr<FST<int64_t>> fst);

    std::shared_ptr<BytesRef> next()  override;

    SeekStatus
    seekCeil(std::shared_ptr<BytesRef> text)  override;

    bool seekExact(std::shared_ptr<BytesRef> text)  override;

    void seekExact(int64_t ord)  override;

    std::shared_ptr<BytesRef> term()  override;

    int64_t ord()  override;

    int docFreq()  override;

    int64_t totalTermFreq()  override;

    std::shared_ptr<PostingsEnum>
    postings(std::shared_ptr<PostingsEnum> reuse,
             int flags)  override;

  protected:
    std::shared_ptr<FSTTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<FSTTermsEnum>(
          TermsEnum::shared_from_this());
    }
  };

protected:
  std::shared_ptr<MemoryDocValuesProducer> shared_from_this()
  {
    return std::static_pointer_cast<MemoryDocValuesProducer>(
        org.apache.lucene.codecs.DocValuesProducer::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::memory
