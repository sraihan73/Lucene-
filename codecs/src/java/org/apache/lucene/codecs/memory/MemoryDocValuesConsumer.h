#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesConsumer.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/store/ByteArrayDataOutput.h"

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

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.BLOCK_COMPRESSED;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.BLOCK_SIZE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.BYTES;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.DELTA_COMPRESSED;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.codecs.memory.MemoryDocValuesProducer.FST;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.GCD_COMPRESSED;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.NUMBER;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.SORTED_NUMERIC;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.SORTED_NUMERIC_SINGLETON;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.SORTED_SET;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.SORTED_SET_SINGLETON;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.TABLE_COMPRESSED;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.MemoryDocValuesProducer.VERSION_CURRENT;

/**
 * Writer for {@link MemoryDocValuesFormat}
 */
class MemoryDocValuesConsumer : public DocValuesConsumer
{
  GET_CLASS_NAME(MemoryDocValuesConsumer)
public:
  std::shared_ptr<IndexOutput> data, meta;
  const int maxDoc;
  const float acceptableOverheadRatio;

  MemoryDocValuesConsumer(std::shared_ptr<SegmentWriteState> state,
                          const std::wstring &dataCodec,
                          const std::wstring &dataExtension,
                          const std::wstring &metaCodec,
                          const std::wstring &metaExtension,
                          float acceptableOverheadRatio) ;

  void addNumericField(std::shared_ptr<FieldInfo> field,
                       std::shared_ptr<DocValuesProducer>
                           valuesProducer)  override;

  virtual void addNumericField(std::shared_ptr<FieldInfo> field,
                               std::deque<std::shared_ptr<Number>> &values,
                               bool optimizeStorage) ;

  virtual ~MemoryDocValuesConsumer();

  void addBinaryField(std::shared_ptr<FieldInfo> field,
                      std::shared_ptr<DocValuesProducer>
                          valuesProducer)  override;

private:
  void addBinaryField(
      std::shared_ptr<FieldInfo> field,
      std::deque<std::shared_ptr<BytesRef>> &values) ;

  void
  writeFST(std::shared_ptr<FieldInfo> field,
           std::deque<std::shared_ptr<BytesRef>> &values) ;

  // TODO: in some cases representing missing with minValue-1 wouldn't take up
  // additional space and so on, but this is very simple, and algorithms only
  // check this for values of 0 anyway (doesnt slow down normal decode)
public:
  template <typename T1>
  void writeMissingBitset(std::deque<T1> values) ;

  void addSortedField(std::shared_ptr<FieldInfo> field,
                      std::shared_ptr<DocValuesProducer>
                          valuesProducer)  override;

private:
  void addSortedField(
      std::shared_ptr<FieldInfo> field,
      std::deque<std::shared_ptr<BytesRef>> &values,
      std::deque<std::shared_ptr<Number>> &docToOrd) ;

public:
  void addSortedNumericField(std::shared_ptr<FieldInfo> field,
                             std::shared_ptr<DocValuesProducer>
                                 valuesProducer)  override;

  // note: this might not be the most efficient... but it's fairly simple
  void addSortedSetField(std::shared_ptr<FieldInfo> field,
                         std::shared_ptr<DocValuesProducer>
                             valuesProducer)  override;

private:
  class IterableAnonymousInnerClass
      : public std::enable_shared_from_this<IterableAnonymousInnerClass>,
        public std::deque<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass)
  private:
    std::shared_ptr<MemoryDocValuesConsumer> outerInstance;

    std::deque<std::shared_ptr<Number>> docToOrdCount;
    std::deque<std::shared_ptr<Number>> ords;

  public:
    IterableAnonymousInnerClass(
        std::shared_ptr<MemoryDocValuesConsumer> outerInstance,
        std::deque<std::shared_ptr<Number>> &docToOrdCount,
        std::deque<std::shared_ptr<Number>> &ords);

    std::shared_ptr<Iterator<std::shared_ptr<BytesRef>>> iterator();
  };

  // per-document vint-encoded byte[]
public:
  class SortedSetIterator
      : public std::enable_shared_from_this<SortedSetIterator>,
        public Iterator<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(SortedSetIterator)
  public:
    std::deque<char> buffer = std::deque<char>(10);
    std::shared_ptr<ByteArrayDataOutput> out =
        std::make_shared<ByteArrayDataOutput>();
    std::shared_ptr<BytesRef> ref = std::make_shared<BytesRef>();

    const std::shared_ptr<Iterator<std::shared_ptr<Number>>> counts;
    const std::shared_ptr<Iterator<std::shared_ptr<Number>>> ords;

    SortedSetIterator(std::shared_ptr<Iterator<std::shared_ptr<Number>>> counts,
                      std::shared_ptr<Iterator<std::shared_ptr<Number>>> ords);

    bool hasNext() override;

    std::shared_ptr<BytesRef> next() override;

    // encodes count values to buffer
  private:
    void encodeValues(int count) ;

  public:
    void remove() override;
  };

protected:
  std::shared_ptr<MemoryDocValuesConsumer> shared_from_this()
  {
    return std::static_pointer_cast<MemoryDocValuesConsumer>(
        org.apache.lucene.codecs.DocValuesConsumer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/memory/
