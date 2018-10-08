#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class PostingsFormat;
}

namespace org::apache::lucene::codecs
{
class FieldsConsumer;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class FieldsProducer;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::index
{
class Fields;
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

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using Fields = org::apache::lucene::index::Fields;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

class CrankyPostingsFormat : public PostingsFormat
{
  GET_CLASS_NAME(CrankyPostingsFormat)
public:
  const std::shared_ptr<PostingsFormat> delegate_;
  const std::shared_ptr<Random> random;

  CrankyPostingsFormat(std::shared_ptr<PostingsFormat> delegate_,
                       std::shared_ptr<Random> random);

  std::shared_ptr<FieldsConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<FieldsProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

public:
  class CrankyFieldsConsumer : public FieldsConsumer
  {
    GET_CLASS_NAME(CrankyFieldsConsumer)
  public:
    const std::shared_ptr<FieldsConsumer> delegate_;
    const std::shared_ptr<Random> random;

    CrankyFieldsConsumer(std::shared_ptr<FieldsConsumer> delegate_,
                         std::shared_ptr<Random> random);

    void write(std::shared_ptr<Fields> fields)  override;

    virtual ~CrankyFieldsConsumer();

  protected:
    std::shared_ptr<CrankyFieldsConsumer> shared_from_this()
    {
      return std::static_pointer_cast<CrankyFieldsConsumer>(
          org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<CrankyPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<CrankyPostingsFormat>(
        org.apache.lucene.codecs.PostingsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::cranky
