#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/PointsFormat.h"

#include  "core/src/java/org/apache/lucene/codecs/PointsWriter.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsReader.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/MergeState.h"

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

using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using MergeState = org::apache::lucene::index::MergeState;
using PointValues = org::apache::lucene::index::PointValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Accountable = org::apache::lucene::util::Accountable;

/**
 * Just like the default point format but with additional asserts.
 */

class AssertingPointsFormat final : public PointsFormat
{
  GET_CLASS_NAME(AssertingPointsFormat)
private:
  const std::shared_ptr<PointsFormat> in_;

  /** Create a new AssertingPointsFormat */
public:
  AssertingPointsFormat();

  /**
   * Expert: Create an AssertingPointsFormat.
   * This is only intended to pass special parameters for testing.
   */
  // TODO: can we randomize this a cleaner way? e.g. stored fields and vectors
  // do this with a separate codec...
  AssertingPointsFormat(std::shared_ptr<PointsFormat> in_);

  std::shared_ptr<PointsWriter> fieldsWriter(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<PointsReader> fieldsReader(
      std::shared_ptr<SegmentReadState> state)  override;

public:
  class AssertingPointsReader : public PointsReader
  {
    GET_CLASS_NAME(AssertingPointsReader)
  private:
    const std::shared_ptr<PointsReader> in_;
    const int maxDoc;

  public:
    AssertingPointsReader(int maxDoc, std::shared_ptr<PointsReader> in_);

    virtual ~AssertingPointsReader();

    std::shared_ptr<PointValues>
    getValues(const std::wstring &field)  override;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    void checkIntegrity()  override;

    std::shared_ptr<PointsReader>
    getMergeInstance()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<AssertingPointsReader> shared_from_this()
    {
      return std::static_pointer_cast<AssertingPointsReader>(
          org.apache.lucene.codecs.PointsReader::shared_from_this());
    }
  };

public:
  class AssertingPointsWriter : public PointsWriter
  {
    GET_CLASS_NAME(AssertingPointsWriter)
  private:
    const std::shared_ptr<PointsWriter> in_;

  public:
    AssertingPointsWriter(std::shared_ptr<SegmentWriteState> writeState,
                          std::shared_ptr<PointsWriter> in_);

    void writeField(
        std::shared_ptr<FieldInfo> fieldInfo,
        std::shared_ptr<PointsReader> values)  override;

    void
    merge(std::shared_ptr<MergeState> mergeState)  override;

    void finish()  override;

    virtual ~AssertingPointsWriter();

  protected:
    std::shared_ptr<AssertingPointsWriter> shared_from_this()
    {
      return std::static_pointer_cast<AssertingPointsWriter>(
          org.apache.lucene.codecs.PointsWriter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<AssertingPointsFormat> shared_from_this()
  {
    return std::static_pointer_cast<AssertingPointsFormat>(
        org.apache.lucene.codecs.PointsFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/asserting/
