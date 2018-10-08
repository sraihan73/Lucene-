#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::packed
{
class PackedLongValues;
}

namespace org::apache::lucene::util::packed
{
class Builder;
}
namespace org::apache::lucene::util
{
class Counter;
}
namespace org::apache::lucene::index
{
class DocsWithFieldSet;
}
namespace org::apache::lucene::index
{
class FieldInfo;
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
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
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
class CachedNumericDVs;
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
using Counter = org::apache::lucene::util::Counter;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/** Buffers up pending long per doc, then flushes when
 *  segment flushes. */
class NumericDocValuesWriter : public DocValuesWriter
{
  GET_CLASS_NAME(NumericDocValuesWriter)

private:
  std::shared_ptr<PackedLongValues::Builder> pending;
  std::shared_ptr<PackedLongValues> finalValues;
  const std::shared_ptr<Counter> iwBytesUsed;
  int64_t bytesUsed = 0;
  std::shared_ptr<DocsWithFieldSet> docsWithField;
  const std::shared_ptr<FieldInfo> fieldInfo;
  int lastDocID = -1;

public:
  NumericDocValuesWriter(std::shared_ptr<FieldInfo> fieldInfo,
                         std::shared_ptr<Counter> iwBytesUsed);

  virtual void addValue(int docID, int64_t value);

private:
  void updateBytesUsed();

public:
  void finish(int maxDoc) override;

  std::shared_ptr<Sorter::DocComparator> getDocComparator(
      int maxDoc,
      std::shared_ptr<SortField> sortField)  override;

  std::shared_ptr<DocIdSetIterator> getDocIdSet() override;

  static std::shared_ptr<SortingLeafReader::CachedNumericDVs> sortDocValues(
      int maxDoc, std::shared_ptr<Sorter::DocMap> sortMap,
      std::shared_ptr<NumericDocValues> oldDocValues) ;

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
    std::shared_ptr<NumericDocValuesWriter> outerInstance;

    std::shared_ptr<PackedLongValues> values;
    std::shared_ptr<
        org::apache::lucene::index::SortingLeafReader::CachedNumericDVs>
        sorted;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<NumericDocValuesWriter> outerInstance,
        std::shared_ptr<PackedLongValues> values,
        std::shared_ptr<
            org::apache::lucene::index::SortingLeafReader::CachedNumericDVs>
            sorted);

    std::shared_ptr<NumericDocValues>
    getNumeric(std::shared_ptr<FieldInfo> fieldInfo) override;

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
  class BufferedNumericDocValues : public NumericDocValues
  {
    GET_CLASS_NAME(BufferedNumericDocValues)
  public:
    const std::shared_ptr<PackedLongValues::Iterator> iter;
    const std::shared_ptr<DocIdSetIterator> docsWithField;

  private:
    int64_t value = 0;

  public:
    BufferedNumericDocValues(std::shared_ptr<PackedLongValues> values,
                             std::shared_ptr<DocIdSetIterator> docsWithFields);

    int docID() override;

    int nextDoc()  override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int64_t longValue() override;

  protected:
    std::shared_ptr<BufferedNumericDocValues> shared_from_this()
    {
      return std::static_pointer_cast<BufferedNumericDocValues>(
          NumericDocValues::shared_from_this());
    }
  };

protected:
  std::shared_ptr<NumericDocValuesWriter> shared_from_this()
  {
    return std::static_pointer_cast<NumericDocValuesWriter>(
        DocValuesWriter::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
