#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesConsumer.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexOutput;
}

namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class DocValuesProducer;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::util
{
class BytesRef;
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

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.BYTES;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.NUMBER;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.SORTED;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.SORTED_NUMERIC;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.SORTED_NUMERIC_SINGLETON;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.SORTED_SET;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.SORTED_SET_SINGLETON;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.memory.DirectDocValuesProducer.VERSION_CURRENT;

/**
 * Writer for {@link DirectDocValuesFormat}
 */

class DirectDocValuesConsumer : public DocValuesConsumer
{
  GET_CLASS_NAME(DirectDocValuesConsumer)
public:
  std::shared_ptr<IndexOutput> data, meta;
  const int maxDoc;

  DirectDocValuesConsumer(std::shared_ptr<SegmentWriteState> state,
                          const std::wstring &dataCodec,
                          const std::wstring &dataExtension,
                          const std::wstring &metaCodec,
                          const std::wstring &metaExtension) ;

  void addNumericField(std::shared_ptr<FieldInfo> field,
                       std::shared_ptr<DocValuesProducer>
                           valuesProducer)  override;

private:
  void addNumericFieldValues(
      std::shared_ptr<FieldInfo> field,
      std::deque<std::shared_ptr<Number>> &values) ;

public:
  virtual ~DirectDocValuesConsumer();

  void addBinaryField(std::shared_ptr<FieldInfo> field,
                      std::shared_ptr<DocValuesProducer>
                          valuesProducer)  override;

private:
  void addBinaryFieldValues(
      std::shared_ptr<FieldInfo> field,
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

  void addSortedNumericField(std::shared_ptr<FieldInfo> field,
                             std::shared_ptr<DocValuesProducer>
                                 valuesProducer)  override;

  // note: this might not be the most efficient... but it's fairly simple
  void addSortedSetField(std::shared_ptr<FieldInfo> field,
                         std::shared_ptr<DocValuesProducer>
                             valuesProducer)  override;

  /**
   * Just aggregates the count values so they become
   * "addresses", and adds one more value in the end
   * (the final sum)
   */
private:
  std::deque<std::shared_ptr<Number>>
  countToAddressIterator(std::deque<std::shared_ptr<Number>> &counts);

private:
  class IterableAnonymousInnerClass
      : public std::enable_shared_from_this<IterableAnonymousInnerClass>,
        public std::deque<std::shared_ptr<Number>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass)
  private:
    std::shared_ptr<DirectDocValuesConsumer> outerInstance;

    std::deque<std::shared_ptr<Number>> counts;

  public:
    IterableAnonymousInnerClass(
        std::shared_ptr<DirectDocValuesConsumer> outerInstance,
        std::deque<std::shared_ptr<Number>> &counts);

    std::shared_ptr<Iterator<std::shared_ptr<Number>>> iterator();

  private:
    class IteratorAnonymousInnerClass
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
          public Iterator<std::shared_ptr<Number>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass)
    private:
      std::shared_ptr<IterableAnonymousInnerClass> outerInstance;

      std::deque<std::shared_ptr<Number>>::const_iterator iter;

    public:
      IteratorAnonymousInnerClass(
          std::shared_ptr<IterableAnonymousInnerClass> outerInstance,
          std::deque<Number>::const_iterator iter);

      int64_t sum = 0;
      bool ended = false;

      bool hasNext();

      std::shared_ptr<Number> next();

      void remove();
    };
  };

protected:
  std::shared_ptr<DirectDocValuesConsumer> shared_from_this()
  {
    return std::static_pointer_cast<DirectDocValuesConsumer>(
        org.apache.lucene.codecs.DocValuesConsumer::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::memory
