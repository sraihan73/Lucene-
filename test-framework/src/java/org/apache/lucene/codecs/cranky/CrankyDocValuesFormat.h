#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class DocValuesFormat;
}

namespace org::apache::lucene::codecs
{
class DocValuesConsumer;
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
class SegmentReadState;
}
namespace org::apache::lucene::index
{
class FieldInfo;
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
namespace org::apache::lucene::codecs::cranky
{

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

class CrankyDocValuesFormat : public DocValuesFormat
{
  GET_CLASS_NAME(CrankyDocValuesFormat)
public:
  const std::shared_ptr<DocValuesFormat> delegate_;
  const std::shared_ptr<Random> random;

  CrankyDocValuesFormat(std::shared_ptr<DocValuesFormat> delegate_,
                        std::shared_ptr<Random> random);

  std::shared_ptr<DocValuesConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<DocValuesProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

public:
  class CrankyDocValuesConsumer : public DocValuesConsumer
  {
    GET_CLASS_NAME(CrankyDocValuesConsumer)
  public:
    const std::shared_ptr<DocValuesConsumer> delegate_;
    const std::shared_ptr<Random> random;

    CrankyDocValuesConsumer(std::shared_ptr<DocValuesConsumer> delegate_,
                            std::shared_ptr<Random> random);

    virtual ~CrankyDocValuesConsumer();

    void addNumericField(std::shared_ptr<FieldInfo> field,
                         std::shared_ptr<DocValuesProducer>
                             valuesProducer)  override;

    void addBinaryField(std::shared_ptr<FieldInfo> field,
                        std::shared_ptr<DocValuesProducer>
                            valuesProducer)  override;

    void addSortedField(std::shared_ptr<FieldInfo> field,
                        std::shared_ptr<DocValuesProducer>
                            valuesProducer)  override;

    void addSortedNumericField(std::shared_ptr<FieldInfo> field,
                               std::shared_ptr<DocValuesProducer>
                                   valuesProducer)  override;

    void addSortedSetField(std::shared_ptr<FieldInfo> field,
                           std::shared_ptr<DocValuesProducer>
                               valuesProducer)  override;

  protected:
    std::shared_ptr<CrankyDocValuesConsumer> shared_from_this()
    {
      return std::static_pointer_cast<CrankyDocValuesConsumer>(
          org.apache.lucene.codecs.DocValuesConsumer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<CrankyDocValuesFormat> shared_from_this()
  {
    return std::static_pointer_cast<CrankyDocValuesFormat>(
        org.apache.lucene.codecs.DocValuesFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::cranky
