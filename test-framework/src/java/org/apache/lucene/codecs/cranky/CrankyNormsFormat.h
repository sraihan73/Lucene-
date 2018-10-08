#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/NormsFormat.h"

#include  "core/src/java/org/apache/lucene/codecs/NormsConsumer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/NormsProducer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"

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
namespace org::apache::lucene::codecs::cranky
{

using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

class CrankyNormsFormat : public NormsFormat
{
  GET_CLASS_NAME(CrankyNormsFormat)
public:
  const std::shared_ptr<NormsFormat> delegate_;
  const std::shared_ptr<Random> random;

  CrankyNormsFormat(std::shared_ptr<NormsFormat> delegate_,
                    std::shared_ptr<Random> random);

  std::shared_ptr<NormsConsumer> normsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<NormsProducer> normsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

public:
  class CrankyNormsConsumer : public NormsConsumer
  {
    GET_CLASS_NAME(CrankyNormsConsumer)
  public:
    const std::shared_ptr<NormsConsumer> delegate_;
    const std::shared_ptr<Random> random;

    CrankyNormsConsumer(std::shared_ptr<NormsConsumer> delegate_,
                        std::shared_ptr<Random> random);

    virtual ~CrankyNormsConsumer();

    void addNormsField(std::shared_ptr<FieldInfo> field,
                       std::shared_ptr<NormsProducer>
                           valuesProducer)  override;

  protected:
    std::shared_ptr<CrankyNormsConsumer> shared_from_this()
    {
      return std::static_pointer_cast<CrankyNormsConsumer>(
          org.apache.lucene.codecs.NormsConsumer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<CrankyNormsFormat> shared_from_this()
  {
    return std::static_pointer_cast<CrankyNormsFormat>(
        org.apache.lucene.codecs.NormsFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/cranky/
