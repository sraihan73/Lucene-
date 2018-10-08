#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

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
class Terms;
}
namespace org::apache::lucene::util
{
class Accountable;
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
namespace org::apache::lucene::codecs::asserting
{

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using Fields = org::apache::lucene::index::Fields;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using Accountable = org::apache::lucene::util::Accountable;
using TestUtil = org::apache::lucene::util::TestUtil;

/**
 * Just like the default postings format but with additional asserts.
 */
class AssertingPostingsFormat final : public PostingsFormat
{
  GET_CLASS_NAME(AssertingPostingsFormat)
private:
  const std::shared_ptr<PostingsFormat> in_ =
      TestUtil::getDefaultPostingsFormat();

public:
  AssertingPostingsFormat();

  std::shared_ptr<FieldsConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<FieldsProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

public:
  class AssertingFieldsProducer : public FieldsProducer
  {
    GET_CLASS_NAME(AssertingFieldsProducer)
  private:
    const std::shared_ptr<FieldsProducer> in_;

  public:
    AssertingFieldsProducer(std::shared_ptr<FieldsProducer> in_);

    virtual ~AssertingFieldsProducer();

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

    int size() override;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    void checkIntegrity()  override;

    std::shared_ptr<FieldsProducer>
    getMergeInstance()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<AssertingFieldsProducer> shared_from_this()
    {
      return std::static_pointer_cast<AssertingFieldsProducer>(
          org.apache.lucene.codecs.FieldsProducer::shared_from_this());
    }
  };

public:
  class AssertingFieldsConsumer : public FieldsConsumer
  {
    GET_CLASS_NAME(AssertingFieldsConsumer)
  private:
    const std::shared_ptr<FieldsConsumer> in_;
    const std::shared_ptr<SegmentWriteState> writeState;

  public:
    AssertingFieldsConsumer(std::shared_ptr<SegmentWriteState> writeState,
                            std::shared_ptr<FieldsConsumer> in_);

    void write(std::shared_ptr<Fields> fields)  override;

    virtual ~AssertingFieldsConsumer();

  protected:
    std::shared_ptr<AssertingFieldsConsumer> shared_from_this()
    {
      return std::static_pointer_cast<AssertingFieldsConsumer>(
          org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<AssertingPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<AssertingPostingsFormat>(
        org.apache.lucene.codecs.PostingsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::asserting
