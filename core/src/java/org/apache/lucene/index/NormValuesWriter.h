#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class DocsWithFieldSet;
}

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
class FieldInfo;
}
namespace org::apache::lucene::codecs
{
class NormsConsumer;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
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
class CachedNumericDVs;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
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

using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Counter = org::apache::lucene::util::Counter;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;

/** Buffers up pending long per doc, then flushes when
 *  segment flushes. */
class NormValuesWriter : public std::enable_shared_from_this<NormValuesWriter>
{
  GET_CLASS_NAME(NormValuesWriter)

private:
  std::shared_ptr<DocsWithFieldSet> docsWithField;
  std::shared_ptr<PackedLongValues::Builder> pending;
  const std::shared_ptr<Counter> iwBytesUsed;
  int64_t bytesUsed = 0;
  const std::shared_ptr<FieldInfo> fieldInfo;
  int lastDocID = -1;

public:
  NormValuesWriter(std::shared_ptr<FieldInfo> fieldInfo,
                   std::shared_ptr<Counter> iwBytesUsed);

  virtual void addValue(int docID, int64_t value);

private:
  void updateBytesUsed();

public:
  virtual void finish(int maxDoc);

  virtual void
  flush(std::shared_ptr<SegmentWriteState> state,
        std::shared_ptr<Sorter::DocMap> sortMap,
        std::shared_ptr<NormsConsumer> normsConsumer) ;

private:
  class NormsProducerAnonymousInnerClass : public NormsProducer
  {
    GET_CLASS_NAME(NormsProducerAnonymousInnerClass)
  private:
    std::shared_ptr<NormValuesWriter> outerInstance;

    std::shared_ptr<PackedLongValues> values;
    std::shared_ptr<
        org::apache::lucene::index::SortingLeafReader::CachedNumericDVs>
        sorted;

  public:
    NormsProducerAnonymousInnerClass(
        std::shared_ptr<NormValuesWriter> outerInstance,
        std::shared_ptr<PackedLongValues> values,
        std::shared_ptr<
            org::apache::lucene::index::SortingLeafReader::CachedNumericDVs>
            sorted);

    std::shared_ptr<NumericDocValues>
    getNorms(std::shared_ptr<FieldInfo> fieldInfo2) override;

    void checkIntegrity() override;

    virtual ~NormsProducerAnonymousInnerClass();

    int64_t ramBytesUsed() override;

  protected:
    std::shared_ptr<NormsProducerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<NormsProducerAnonymousInnerClass>(
          org.apache.lucene.codecs.NormsProducer::shared_from_this());
    }
  };

  // TODO: norms should only visit docs that had a field indexed!!

  // iterates over the values we have in ram
private:
  class BufferedNorms : public NumericDocValues
  {
    GET_CLASS_NAME(BufferedNorms)
  public:
    const std::shared_ptr<PackedLongValues::Iterator> iter;
    const std::shared_ptr<DocIdSetIterator> docsWithField;

  private:
    int64_t value = 0;

  public:
    BufferedNorms(std::shared_ptr<PackedLongValues> values,
                  std::shared_ptr<DocIdSetIterator> docsWithFields);

    int docID() override;

    int nextDoc()  override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int64_t longValue() override;

  protected:
    std::shared_ptr<BufferedNorms> shared_from_this()
    {
      return std::static_pointer_cast<BufferedNorms>(
          NumericDocValues::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::index
