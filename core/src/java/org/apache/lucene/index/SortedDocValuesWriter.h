#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRefHash;
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
namespace org::apache::lucene::util
{
class Counter;
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
class Sorter;
}
namespace org::apache::lucene::index
{
class DocComparator;
}
namespace org::apache::lucene::search
{
class SortField;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
}
namespace org::apache::lucene::index
{
class DocMap;
}
namespace org::apache::lucene::codecs
{
class DocValuesConsumer;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::util::packed
{
class Iterator;
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

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SortField = org::apache::lucene::search::SortField;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using Counter = org::apache::lucene::util::Counter;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_SIZE;

/** Buffers up pending byte[] per doc, deref and sorting via
 *  int ord, then flushes when segment flushes. */
class SortedDocValuesWriter : public DocValuesWriter
{
  GET_CLASS_NAME(SortedDocValuesWriter)
public:
  const std::shared_ptr<BytesRefHash> hash;

private:
  std::shared_ptr<PackedLongValues::Builder> pending;
  std::shared_ptr<DocsWithFieldSet> docsWithField;
  const std::shared_ptr<Counter> iwBytesUsed;
  int64_t bytesUsed =
      0; // this currently only tracks differences in 'pending'
  const std::shared_ptr<FieldInfo> fieldInfo;
  int lastDocID = -1;

  std::shared_ptr<PackedLongValues> finalOrds;
  std::deque<int> finalSortedValues;
  std::deque<int> finalOrdMap;

public:
  SortedDocValuesWriter(std::shared_ptr<FieldInfo> fieldInfo,
                        std::shared_ptr<Counter> iwBytesUsed);

  virtual void addValue(int docID, std::shared_ptr<BytesRef> value);

  void finish(int maxDoc) override;

private:
  void addOneValue(std::shared_ptr<BytesRef> value);

  void updateBytesUsed();

public:
  std::shared_ptr<Sorter::DocComparator> getDocComparator(
      int maxDoc,
      std::shared_ptr<SortField> sortField)  override;

private:
  std::deque<int>
  sortDocValues(int maxDoc, std::shared_ptr<Sorter::DocMap> sortMap,
                std::shared_ptr<SortedDocValues> oldValues) ;

public:
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
    std::shared_ptr<SortedDocValuesWriter> outerInstance;

    int valueCount = 0;
    std::shared_ptr<PackedLongValues> ords;
    std::deque<int> sortedValues;
    std::deque<int> ordMap;
    std::deque<int> sorted;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<SortedDocValuesWriter> outerInstance, int valueCount,
        std::shared_ptr<PackedLongValues> ords, std::deque<int> &sortedValues,
        std::deque<int> &ordMap, std::deque<int> &sorted);

    std::shared_ptr<SortedDocValues>
    getSorted(std::shared_ptr<FieldInfo> fieldInfoIn) override;

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass>(
          EmptyDocValuesProducer::shared_from_this());
    }
  };

private:
  class BufferedSortedDocValues : public SortedDocValues
  {
    GET_CLASS_NAME(BufferedSortedDocValues)
  public:
    const std::shared_ptr<BytesRefHash> hash;
    const std::shared_ptr<BytesRef> scratch = std::make_shared<BytesRef>();
    std::deque<int> const sortedValues;
    std::deque<int> const ordMap;
    const int valueCount;

  private:
    int ord = 0;

  public:
    const std::shared_ptr<PackedLongValues::Iterator> iter;
    const std::shared_ptr<DocIdSetIterator> docsWithField;

    BufferedSortedDocValues(std::shared_ptr<BytesRefHash> hash, int valueCount,
                            std::shared_ptr<PackedLongValues> docToOrd,
                            std::deque<int> &sortedValues,
                            std::deque<int> &ordMap,
                            std::shared_ptr<DocIdSetIterator> docsWithField);

    int docID() override;

    int nextDoc()  override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int ordValue() override;

    std::shared_ptr<BytesRef> lookupOrd(int ord) override;

    int getValueCount() override;

  protected:
    std::shared_ptr<BufferedSortedDocValues> shared_from_this()
    {
      return std::static_pointer_cast<BufferedSortedDocValues>(
          SortedDocValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<DocIdSetIterator> getDocIdSet() override;

protected:
  std::shared_ptr<SortedDocValuesWriter> shared_from_this()
  {
    return std::static_pointer_cast<SortedDocValuesWriter>(
        DocValuesWriter::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
