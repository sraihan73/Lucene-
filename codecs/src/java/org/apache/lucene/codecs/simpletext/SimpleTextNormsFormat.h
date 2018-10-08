#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/NormsConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/NormsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/NormsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/EmptyDocValuesProducer.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::simpletext
{
class SimpleTextDocValuesWriter;
}

namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs::simpletext
{
class SimpleTextDocValuesReader;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::util
{
class Accountable;
}
namespace org::apache::lucene::codecs
{
class NormsProducer;
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
namespace org::apache::lucene::codecs::simpletext
{

using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Accountable = org::apache::lucene::util::Accountable;

/**
 * plain-text norms format.
 * <p>
 * <b>FOR RECREATIONAL USE ONLY</b>
 *
 * @lucene.experimental
 */
class SimpleTextNormsFormat : public NormsFormat
{
  GET_CLASS_NAME(SimpleTextNormsFormat)
private:
  static const std::wstring NORMS_SEG_EXTENSION;

public:
  std::shared_ptr<NormsConsumer> normsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<NormsProducer> normsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

  /**
   * Reads plain-text norms.
   * <p>
   * <b>FOR RECREATIONAL USE ONLY</b>
   *
   * @lucene.experimental
   */
public:
  class SimpleTextNormsProducer : public NormsProducer
  {
    GET_CLASS_NAME(SimpleTextNormsProducer)
  private:
    const std::shared_ptr<SimpleTextDocValuesReader> impl;

  public:
    SimpleTextNormsProducer(std::shared_ptr<SegmentReadState> state) throw(
        IOException);

    std::shared_ptr<NumericDocValues>
    getNorms(std::shared_ptr<FieldInfo> field)  override;

    virtual ~SimpleTextNormsProducer();

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    void checkIntegrity()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<SimpleTextNormsProducer> shared_from_this()
    {
      return std::static_pointer_cast<SimpleTextNormsProducer>(
          org.apache.lucene.codecs.NormsProducer::shared_from_this());
    }
  };

  /**
   * Writes plain-text norms.
   * <p>
   * <b>FOR RECREATIONAL USE ONLY</b>
   *
   * @lucene.experimental
   */
public:
  class SimpleTextNormsConsumer : public NormsConsumer
  {
    GET_CLASS_NAME(SimpleTextNormsConsumer)
  private:
    const std::shared_ptr<SimpleTextDocValuesWriter> impl;

  public:
    SimpleTextNormsConsumer(std::shared_ptr<SegmentWriteState> state) throw(
        IOException);

    void addNormsField(std::shared_ptr<FieldInfo> field,
                       std::shared_ptr<NormsProducer>
                           normsProducer)  override;

  private:
    class EmptyDocValuesProducerAnonymousInnerClass
        : public EmptyDocValuesProducer
    {
      GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass)
    private:
      std::shared_ptr<SimpleTextNormsConsumer> outerInstance;

      std::shared_ptr<FieldInfo> field;
      std::shared_ptr<NormsProducer> normsProducer;

    public:
      EmptyDocValuesProducerAnonymousInnerClass(
          std::shared_ptr<SimpleTextNormsConsumer> outerInstance,
          std::shared_ptr<FieldInfo> field,
          std::shared_ptr<NormsProducer> normsProducer);

      std::shared_ptr<NumericDocValues>
      getNumeric(std::shared_ptr<FieldInfo> field)  override;

    protected:
      std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
      shared_from_this()
      {
        return std::static_pointer_cast<
            EmptyDocValuesProducerAnonymousInnerClass>(
            org.apache.lucene.index.EmptyDocValuesProducer::shared_from_this());
      }
    };

  public:
    virtual ~SimpleTextNormsConsumer();

  protected:
    std::shared_ptr<SimpleTextNormsConsumer> shared_from_this()
    {
      return std::static_pointer_cast<SimpleTextNormsConsumer>(
          org.apache.lucene.codecs.NormsConsumer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SimpleTextNormsFormat> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextNormsFormat>(
        org.apache.lucene.codecs.NormsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::simpletext
