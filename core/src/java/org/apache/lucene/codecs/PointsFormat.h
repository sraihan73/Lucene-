#pragma once
#include "PointsReader.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/PointsWriter.h"

#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"

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
namespace org::apache::lucene::codecs
{

using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

/**
 * Encodes/decodes indexed points.
 *
 * @lucene.experimental */
class PointsFormat : public std::enable_shared_from_this<PointsFormat>
{
  GET_CLASS_NAME(PointsFormat)

  /**
   * Creates a new point format.
   */
protected:
  PointsFormat();

  /** Writes a new segment */
public:
  virtual std::shared_ptr<PointsWriter>
  fieldsWriter(std::shared_ptr<SegmentWriteState> state) = 0;

  /** Reads a segment.  NOTE: by the time this call
   *  returns, it must hold open any files it will need to
   *  use; else, those files may be deleted.
   *  Additionally, required files may be deleted during the execution of
   *  this call before there is a chance to open them. Under these
   *  circumstances an IOException should be thrown by the implementation.
   *  IOExceptions are expected and will automatically cause a retry of the
   *  segment opening logic with the newly revised segments.
   *  */
  virtual std::shared_ptr<PointsReader>
  fieldsReader(std::shared_ptr<SegmentReadState> state) = 0;

  /** A {@code PointsFormat} that has nothing indexed */
  static const std::shared_ptr<PointsFormat> EMPTY;

private:
  class PointsFormatAnonymousInnerClass;
};

} // namespace org::apache::lucene::codecs
class PointsFormat::PointsFormatAnonymousInnerClass : public PointsFormat
{
  GET_CLASS_NAME(PointsFormat::PointsFormatAnonymousInnerClass)
public:
  PointsFormatAnonymousInnerClass();

  std::shared_ptr<PointsWriter>
  fieldsWriter(std::shared_ptr<SegmentWriteState> state) override;

  std::shared_ptr<PointsReader>
  fieldsReader(std::shared_ptr<SegmentReadState> state) override;

private:
  class PointsReaderAnonymousInnerClass : public PointsReader
  {
    GET_CLASS_NAME(PointsReaderAnonymousInnerClass)
  private:
    std::shared_ptr<PointsFormatAnonymousInnerClass> outerInstance;

  public:
    PointsReaderAnonymousInnerClass(
        std::shared_ptr<PointsFormatAnonymousInnerClass> outerInstance);

    virtual ~PointsReaderAnonymousInnerClass();

    int64_t ramBytesUsed() override;

    void checkIntegrity() override;

    std::shared_ptr<PointValues> getValues(const std::wstring &field) override;

  protected:
    std::shared_ptr<PointsReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PointsReaderAnonymousInnerClass>(
          PointsReader::shared_from_this());
    }
  };

protected:
  std::shared_ptr<PointsFormatAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<PointsFormatAnonymousInnerClass>(
        PointsFormat::shared_from_this());
  }
};
