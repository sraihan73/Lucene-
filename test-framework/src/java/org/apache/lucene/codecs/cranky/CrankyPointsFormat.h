#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class PointsFormat;
}

namespace org::apache::lucene::codecs
{
class PointsWriter;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class PointsReader;
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
class MergeState;
}
namespace org::apache::lucene::index
{
class PointValues;
}
namespace org::apache::lucene::index
{
class IntersectVisitor;
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

using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using MergeState = org::apache::lucene::index::MergeState;
using PointValues = org::apache::lucene::index::PointValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

class CrankyPointsFormat : public PointsFormat
{
  GET_CLASS_NAME(CrankyPointsFormat)
public:
  std::shared_ptr<PointsFormat> delegate_;
  std::shared_ptr<Random> random;

  CrankyPointsFormat(std::shared_ptr<PointsFormat> delegate_,
                     std::shared_ptr<Random> random);

  std::shared_ptr<PointsWriter> fieldsWriter(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<PointsReader> fieldsReader(
      std::shared_ptr<SegmentReadState> state)  override;

public:
  class CrankyPointsWriter : public PointsWriter
  {
    GET_CLASS_NAME(CrankyPointsWriter)
  public:
    const std::shared_ptr<PointsWriter> delegate_;
    const std::shared_ptr<Random> random;

    CrankyPointsWriter(std::shared_ptr<PointsWriter> delegate_,
                       std::shared_ptr<Random> random);

    void writeField(
        std::shared_ptr<FieldInfo> fieldInfo,
        std::shared_ptr<PointsReader> values)  override;

    void finish()  override;

    void
    merge(std::shared_ptr<MergeState> mergeState)  override;

    virtual ~CrankyPointsWriter();

  protected:
    std::shared_ptr<CrankyPointsWriter> shared_from_this()
    {
      return std::static_pointer_cast<CrankyPointsWriter>(
          org.apache.lucene.codecs.PointsWriter::shared_from_this());
    }
  };

public:
  class CrankyPointsReader : public PointsReader
  {
    GET_CLASS_NAME(CrankyPointsReader)
  public:
    const std::shared_ptr<PointsReader> delegate_;
    const std::shared_ptr<Random> random;
    CrankyPointsReader(std::shared_ptr<PointsReader> delegate_,
                       std::shared_ptr<Random> random);

    void checkIntegrity()  override;

    std::shared_ptr<PointValues>
    getValues(const std::wstring &fieldName)  override;

  private:
    class PointValuesAnonymousInnerClass : public PointValues
    {
      GET_CLASS_NAME(PointValuesAnonymousInnerClass)
    private:
      std::shared_ptr<CrankyPointsReader> outerInstance;

      std::shared_ptr<PointValues> delegate_;

    public:
      PointValuesAnonymousInnerClass(
          std::shared_ptr<CrankyPointsReader> outerInstance,
          std::shared_ptr<PointValues> delegate_);

      void
      intersect(std::shared_ptr<PointValues::IntersectVisitor> visitor) throw(
          IOException) override;

      int64_t estimatePointCount(
          std::shared_ptr<PointValues::IntersectVisitor> visitor) override;

      std::deque<char> getMinPackedValue()  override;

      std::deque<char> getMaxPackedValue()  override;

      int getNumDimensions()  override;

      int getBytesPerDimension()  override;

      int64_t size() override;

      int getDocCount() override;

    protected:
      std::shared_ptr<PointValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<PointValuesAnonymousInnerClass>(
            org.apache.lucene.index.PointValues::shared_from_this());
      }
    };

  public:
    virtual ~CrankyPointsReader();

    int64_t ramBytesUsed() override;

  protected:
    std::shared_ptr<CrankyPointsReader> shared_from_this()
    {
      return std::static_pointer_cast<CrankyPointsReader>(
          org.apache.lucene.codecs.PointsReader::shared_from_this());
    }
  };

protected:
  std::shared_ptr<CrankyPointsFormat> shared_from_this()
  {
    return std::static_pointer_cast<CrankyPointsFormat>(
        org.apache.lucene.codecs.PointsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::cranky
