#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/NormsFormat.h"

#include  "core/src/java/org/apache/lucene/codecs/NormsConsumer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/NormsProducer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"

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
namespace org::apache::lucene::codecs::asserting
{

using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Accountable = org::apache::lucene::util::Accountable;
using TestUtil = org::apache::lucene::util::TestUtil;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * Just like the default but with additional asserts.
 */
class AssertingNormsFormat : public NormsFormat
{
  GET_CLASS_NAME(AssertingNormsFormat)
private:
  const std::shared_ptr<NormsFormat> in_ =
      TestUtil::getDefaultCodec()->normsFormat();

public:
  std::shared_ptr<NormsConsumer> normsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<NormsProducer> normsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

public:
  class AssertingNormsConsumer : public NormsConsumer
  {
    GET_CLASS_NAME(AssertingNormsConsumer)
  private:
    const std::shared_ptr<NormsConsumer> in_;
    const int maxDoc;

  public:
    AssertingNormsConsumer(std::shared_ptr<NormsConsumer> in_, int maxDoc);

    void addNormsField(std::shared_ptr<FieldInfo> field,
                       std::shared_ptr<NormsProducer>
                           valuesProducer)  override;

    virtual ~AssertingNormsConsumer();

  protected:
    std::shared_ptr<AssertingNormsConsumer> shared_from_this()
    {
      return std::static_pointer_cast<AssertingNormsConsumer>(
          org.apache.lucene.codecs.NormsConsumer::shared_from_this());
    }
  };

public:
  class AssertingNormsProducer : public NormsProducer
  {
    GET_CLASS_NAME(AssertingNormsProducer)
  private:
    const std::shared_ptr<NormsProducer> in_;
    const int maxDoc;

  public:
    AssertingNormsProducer(std::shared_ptr<NormsProducer> in_, int maxDoc);

    std::shared_ptr<NumericDocValues>
    getNorms(std::shared_ptr<FieldInfo> field)  override;

    virtual ~AssertingNormsProducer();

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    void checkIntegrity()  override;

    std::shared_ptr<NormsProducer>
    getMergeInstance()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<AssertingNormsProducer> shared_from_this()
    {
      return std::static_pointer_cast<AssertingNormsProducer>(
          org.apache.lucene.codecs.NormsProducer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<AssertingNormsFormat> shared_from_this()
  {
    return std::static_pointer_cast<AssertingNormsFormat>(
        org.apache.lucene.codecs.NormsFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/asserting/
