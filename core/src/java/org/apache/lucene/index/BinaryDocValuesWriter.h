#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class PagedBytes;
}

namespace org::apache::lucene::store
{
class DataOutput;
}
namespace org::apache::lucene::util
{
class Counter;
}
namespace org::apache::lucene::util::packed
{
class PackedLongValues;
}
namespace org::apache::lucene::util::packed
{
class Builder;
}
namespace org::apache::lucene::index
{
class DocsWithFieldSet;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class BinaryDocValues;
}
namespace org::apache::lucene::index
{
class Sorter;
}
namespace org::apache::lucene::index
{
class DocMap;
}
namespace org::apache::lucene::index
{
class SortingLeafReader;
}
namespace org::apache::lucene::index
{
class CachedBinaryDVs;
}
namespace org::apache::lucene::index
{
class DocComparator;
}
namespace org::apache::lucene::search
{
class SortField;
}
namespace org::apache::lucene::codecs
{
class DocValuesConsumer;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::util::packed
{
class Iterator;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::store
{
class DataInput;
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

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SortField = org::apache::lucene::search::SortField;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using Counter = org::apache::lucene::util::Counter;
using PagedBytes = org::apache::lucene::util::PagedBytes;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/** Buffers up pending byte[] per doc, then flushes when
 *  segment flushes. */
class BinaryDocValuesWriter : public DocValuesWriter
{
  GET_CLASS_NAME(BinaryDocValuesWriter)

  /** Maximum length for a binary field. */
private:
  static const int MAX_LENGTH = ArrayUtil::MAX_ARRAY_LENGTH;

  // 32 KB block sizes for PagedBytes storage:
  static constexpr int BLOCK_BITS = 15;

  const std::shared_ptr<PagedBytes> bytes;
  const std::shared_ptr<DataOutput> bytesOut;

  const std::shared_ptr<Counter> iwBytesUsed;
  const std::shared_ptr<PackedLongValues::Builder> lengths;
  std::shared_ptr<DocsWithFieldSet> docsWithField;
  const std::shared_ptr<FieldInfo> fieldInfo;
  int64_t bytesUsed = 0;
  int lastDocID = -1;
  int maxLength = 0;

public:
  BinaryDocValuesWriter(std::shared_ptr<FieldInfo> fieldInfo,
                        std::shared_ptr<Counter> iwBytesUsed);

  virtual void addValue(int docID, std::shared_ptr<BytesRef> value);

private:
  void updateBytesUsed();

public:
  void finish(int maxDoc) override;

private:
  std::shared_ptr<SortingLeafReader::CachedBinaryDVs>
  sortDocValues(int maxDoc, std::shared_ptr<Sorter::DocMap> sortMap,
                std::shared_ptr<BinaryDocValues> oldValues) ;

public:
  std::shared_ptr<Sorter::DocComparator> getDocComparator(
      int numDoc,
      std::shared_ptr<SortField> sortField)  override;

  void flush(std::shared_ptr<SegmentWriteState> state,
             std::shared_ptr<Sorter::DocMap> sortMap,
             std::shared_ptr<DocValuesConsumer> dvConsumer) 
      override;

private:
  class EmptyDocValuesProducerAnonymousInnerClass
      : public EmptyDocValuesProducer
  {
    GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass)
  private:
    std::shared_ptr<BinaryDocValuesWriter> outerInstance;

    std::shared_ptr<PackedLongValues> lengths;
    std::shared_ptr<
        org::apache::lucene::index::SortingLeafReader::CachedBinaryDVs>
        sorted;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<BinaryDocValuesWriter> outerInstance,
        std::shared_ptr<PackedLongValues> lengths,
        std::shared_ptr<
            org::apache::lucene::index::SortingLeafReader::CachedBinaryDVs>
            sorted);

    std::shared_ptr<BinaryDocValues>
    getBinary(std::shared_ptr<FieldInfo> fieldInfoIn) override;

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass>(
          EmptyDocValuesProducer::shared_from_this());
    }
  };

  // iterates over the values we have in ram
private:
  class BufferedBinaryDocValues : public BinaryDocValues
  {
    GET_CLASS_NAME(BufferedBinaryDocValues)
  public:
    const std::shared_ptr<BytesRefBuilder> value;
    const std::shared_ptr<PackedLongValues::Iterator> lengthsIterator;
    const std::shared_ptr<DocIdSetIterator> docsWithField;
    const std::shared_ptr<DataInput> bytesIterator;

    BufferedBinaryDocValues(std::shared_ptr<PackedLongValues> lengths,
                            int maxLength,
                            std::shared_ptr<DataInput> bytesIterator,
                            std::shared_ptr<DocIdSetIterator> docsWithFields);

    int docID() override;

    int nextDoc()  override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    std::shared_ptr<BytesRef> binaryValue() override;

  protected:
    std::shared_ptr<BufferedBinaryDocValues> shared_from_this()
    {
      return std::static_pointer_cast<BufferedBinaryDocValues>(
          BinaryDocValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<DocIdSetIterator> getDocIdSet() override;

protected:
  std::shared_ptr<BinaryDocValuesWriter> shared_from_this()
  {
    return std::static_pointer_cast<BinaryDocValuesWriter>(
        DocValuesWriter::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
