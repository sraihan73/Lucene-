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
class SortedSetDocValues;
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
//    import static org.apache.lucene.index.SortedSetDocValues.NO_MORE_ORDS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_SIZE;

/** Buffers up pending byte[]s per doc, deref and sorting via
 *  int ord, then flushes when segment flushes. */
class SortedSetDocValuesWriter : public DocValuesWriter
{
  GET_CLASS_NAME(SortedSetDocValuesWriter)
public:
  const std::shared_ptr<BytesRefHash> hash;

private:
  std::shared_ptr<PackedLongValues::Builder> pending; // stream of all termIDs
  std::shared_ptr<PackedLongValues::Builder> pendingCounts; // termIDs per doc
  std::shared_ptr<DocsWithFieldSet> docsWithField;
  const std::shared_ptr<Counter> iwBytesUsed;
  int64_t bytesUsed =
      0; // this only tracks differences in 'pending' and 'pendingCounts'
  const std::shared_ptr<FieldInfo> fieldInfo;
  int currentDoc = -1;
  std::deque<int> currentValues = std::deque<int>(8);
  int currentUpto = 0;
  int maxCount = 0;

  std::shared_ptr<PackedLongValues> finalOrds;
  std::shared_ptr<PackedLongValues> finalOrdCounts;
  std::deque<int> finalSortedValues;
  std::deque<int> finalOrdMap;

public:
  SortedSetDocValuesWriter(std::shared_ptr<FieldInfo> fieldInfo,
                           std::shared_ptr<Counter> iwBytesUsed);

  virtual void addValue(int docID, std::shared_ptr<BytesRef> value);

  // finalize currentDoc: this deduplicates the current term ids
private:
  void finishCurrentDoc();

public:
  void finish(int maxDoc) override;

private:
  void addOneValue(std::shared_ptr<BytesRef> value);

  void updateBytesUsed();

  std::deque<std::deque<int64_t>> sortDocValues(
      int maxDoc, std::shared_ptr<Sorter::DocMap> sortMap,
      std::shared_ptr<SortedSetDocValues> oldValues) ;

public:
  std::shared_ptr<Sorter::DocComparator> getDocComparator(
      int maxDoc,
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
    std::shared_ptr<SortedSetDocValuesWriter> outerInstance;

    std::shared_ptr<PackedLongValues> ords;
    std::shared_ptr<PackedLongValues> ordCounts;
    std::deque<int> sortedValues;
    std::deque<int> ordMap;
    std::deque<std::deque<int64_t>> sorted;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<SortedSetDocValuesWriter> outerInstance,
        std::shared_ptr<PackedLongValues> ords,
        std::shared_ptr<PackedLongValues> ordCounts,
        std::deque<int> &sortedValues, std::deque<int> &ordMap,
        std::deque<std::deque<int64_t>> &sorted);

    std::shared_ptr<SortedSetDocValues>
    getSortedSet(std::shared_ptr<FieldInfo> fieldInfoIn) override;

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
  class BufferedSortedSetDocValues : public SortedSetDocValues
  {
    GET_CLASS_NAME(BufferedSortedSetDocValues)
  public:
    std::deque<int> const sortedValues;
    std::deque<int> const ordMap;
    const std::shared_ptr<BytesRefHash> hash;
    const std::shared_ptr<BytesRef> scratch = std::make_shared<BytesRef>();
    const std::shared_ptr<PackedLongValues::Iterator> ordsIter;
    const std::shared_ptr<PackedLongValues::Iterator> ordCountsIter;
    const std::shared_ptr<DocIdSetIterator> docsWithField;
    std::deque<int> const currentDoc;

  private:
    int ordCount = 0;
    int ordUpto = 0;

  public:
    BufferedSortedSetDocValues(std::deque<int> &sortedValues,
                               std::deque<int> &ordMap,
                               std::shared_ptr<BytesRefHash> hash,
                               std::shared_ptr<PackedLongValues> ords,
                               std::shared_ptr<PackedLongValues> ordCounts,
                               int maxCount,
                               std::shared_ptr<DocIdSetIterator> docsWithField);

    int docID() override;

    int nextDoc()  override;

    int64_t nextOrd() override;

    int64_t cost() override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t getValueCount() override;

    std::shared_ptr<BytesRef> lookupOrd(int64_t ord) override;

  protected:
    std::shared_ptr<BufferedSortedSetDocValues> shared_from_this()
    {
      return std::static_pointer_cast<BufferedSortedSetDocValues>(
          SortedSetDocValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<DocIdSetIterator> getDocIdSet() override;

protected:
  std::shared_ptr<SortedSetDocValuesWriter> shared_from_this()
  {
    return std::static_pointer_cast<SortedSetDocValuesWriter>(
        DocValuesWriter::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
