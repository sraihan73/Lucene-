#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
namespace org::apache::lucene::index
{
class SortedNumericDocValues;
}
namespace org::apache::lucene::store
{
class GrowableByteArrayDataOutput;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene70.Lucene70DocValuesFormat.DIRECT_MONOTONIC_BLOCK_SHIFT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene70.Lucene70DocValuesFormat.NUMERIC_BLOCK_SHIFT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.lucene70.Lucene70DocValuesFormat.NUMERIC_BLOCK_SIZE;

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using GrowableByteArrayDataOutput =
    org::apache::lucene::store::GrowableByteArrayDataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;

/** writer for {@link Lucene70DocValuesFormat} */
class Lucene70DocValuesConsumer final : public DocValuesConsumer
{
  GET_CLASS_NAME(Lucene70DocValuesConsumer)

public:
  std::shared_ptr<IndexOutput> data, meta;
  const int maxDoc;

  /** expert: Creates a new writer */
  Lucene70DocValuesConsumer(
      std::shared_ptr<SegmentWriteState> state, const std::wstring &dataCodec,
      const std::wstring &dataExtension, const std::wstring &metaCodec,
      const std::wstring &metaExtension) ;

  virtual ~Lucene70DocValuesConsumer();

  void addNumericField(std::shared_ptr<FieldInfo> field,
                       std::shared_ptr<DocValuesProducer>
                           valuesProducer)  override;

private:
  class EmptyDocValuesProducerAnonymousInnerClass
      : public EmptyDocValuesProducer
  {
    GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene70DocValuesConsumer> outerInstance;

    std::shared_ptr<FieldInfo> field;
    std::shared_ptr<DocValuesProducer> valuesProducer;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<Lucene70DocValuesConsumer> outerInstance,
        std::shared_ptr<FieldInfo> field,
        std::shared_ptr<DocValuesProducer> valuesProducer);

    std::shared_ptr<SortedNumericDocValues> getSortedNumeric(
        std::shared_ptr<FieldInfo> field)  override;

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass>(
          org.apache.lucene.index.EmptyDocValuesProducer::shared_from_this());
    }
  };

private:
  class MinMaxTracker : public std::enable_shared_from_this<MinMaxTracker>
  {
    GET_CLASS_NAME(MinMaxTracker)
  public:
    int64_t min = 0, max = 0, numValues = 0, spaceInBits = 0;

    MinMaxTracker();

  private:
    void reset();

    /** Accumulate a new value. */
  public:
    virtual void update(int64_t v);

    /** Update the required space. */
    virtual void finish();

    /** Update space usage and get ready for accumulating values for the next
     * block. */
    virtual void nextBlock();
  };

private:
  std::deque<int64_t> writeValues(
      std::shared_ptr<FieldInfo> field,
      std::shared_ptr<DocValuesProducer> valuesProducer) ;

  void writeValuesSingleBlock(
      std::shared_ptr<SortedNumericDocValues> values, int64_t numValues,
      int numBitsPerValue, int64_t min, int64_t gcd,
      std::unordered_map<int64_t, int> &encode) ;

  void writeValuesMultipleBlocks(std::shared_ptr<SortedNumericDocValues> values,
                                 int64_t gcd) ;

  void writeBlock(
      std::deque<int64_t> &values, int length, int64_t gcd,
      std::shared_ptr<GrowableByteArrayDataOutput> buffer) ;

public:
  void addBinaryField(std::shared_ptr<FieldInfo> field,
                      std::shared_ptr<DocValuesProducer>
                          valuesProducer)  override;

  void addSortedField(std::shared_ptr<FieldInfo> field,
                      std::shared_ptr<DocValuesProducer>
                          valuesProducer)  override;

private:
  void doAddSortedField(
      std::shared_ptr<FieldInfo> field,
      std::shared_ptr<DocValuesProducer> valuesProducer) ;

  void
  addTermsDict(std::shared_ptr<SortedSetDocValues> values) ;

  void writeTermsIndex(std::shared_ptr<SortedSetDocValues> values) throw(
      IOException);

public:
  void addSortedNumericField(std::shared_ptr<FieldInfo> field,
                             std::shared_ptr<DocValuesProducer>
                                 valuesProducer)  override;

  void addSortedSetField(std::shared_ptr<FieldInfo> field,
                         std::shared_ptr<DocValuesProducer>
                             valuesProducer)  override;

private:
  class EmptyDocValuesProducerAnonymousInnerClass
      : public EmptyDocValuesProducer
  {
    GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass)
  private:
    std::shared_ptr<Lucene70DocValuesConsumer> outerInstance;

    std::shared_ptr<FieldInfo> field;
    std::shared_ptr<DocValuesProducer> valuesProducer;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<Lucene70DocValuesConsumer> outerInstance,
        std::shared_ptr<FieldInfo> field,
        std::shared_ptr<DocValuesProducer> valuesProducer);

    std::shared_ptr<SortedDocValues>
    getSorted(std::shared_ptr<FieldInfo> field)  override;

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass>(
          org.apache.lucene.index.EmptyDocValuesProducer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<Lucene70DocValuesConsumer> shared_from_this()
  {
    return std::static_pointer_cast<Lucene70DocValuesConsumer>(
        org.apache.lucene.codecs.DocValuesConsumer::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene70
