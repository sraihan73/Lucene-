#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class TermVectorsFormat;
}

namespace org::apache::lucene::codecs
{
class TermVectorsReader;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class SegmentInfo;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::codecs
{
class TermVectorsWriter;
}
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::util
{
class Accountable;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::util
{
class BytesRef;
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

using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;

/**
 * Just like the default vectors format but with additional asserts.
 */
class AssertingTermVectorsFormat : public TermVectorsFormat
{
  GET_CLASS_NAME(AssertingTermVectorsFormat)
private:
  const std::shared_ptr<TermVectorsFormat> in_ =
      TestUtil::getDefaultCodec()->termVectorsFormat();

public:
  std::shared_ptr<TermVectorsReader>
  vectorsReader(std::shared_ptr<Directory> directory,
                std::shared_ptr<SegmentInfo> segmentInfo,
                std::shared_ptr<FieldInfos> fieldInfos,
                std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<TermVectorsWriter>
  vectorsWriter(std::shared_ptr<Directory> directory,
                std::shared_ptr<SegmentInfo> segmentInfo,
                std::shared_ptr<IOContext> context)  override;

public:
  class AssertingTermVectorsReader : public TermVectorsReader
  {
    GET_CLASS_NAME(AssertingTermVectorsReader)
  private:
    const std::shared_ptr<TermVectorsReader> in_;

  public:
    AssertingTermVectorsReader(std::shared_ptr<TermVectorsReader> in_);

    virtual ~AssertingTermVectorsReader();

    std::shared_ptr<Fields> get(int doc)  override;

    std::shared_ptr<TermVectorsReader> clone() override;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    void checkIntegrity()  override;

    std::shared_ptr<TermVectorsReader>
    getMergeInstance()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<AssertingTermVectorsReader> shared_from_this()
    {
      return std::static_pointer_cast<AssertingTermVectorsReader>(
          org.apache.lucene.codecs.TermVectorsReader::shared_from_this());
    }
  };

public:
  enum class Status { GET_CLASS_NAME(Status) UNDEFINED, STARTED, FINISHED };

public:
  class AssertingTermVectorsWriter : public TermVectorsWriter
  {
    GET_CLASS_NAME(AssertingTermVectorsWriter)
  private:
    const std::shared_ptr<TermVectorsWriter> in_;
    Status docStatus = static_cast<Status>(0),
           fieldStatus = static_cast<Status>(0),
           termStatus = static_cast<Status>(0);
    int docCount = 0, fieldCount = 0, termCount = 0, positionCount = 0;

  public:
    bool hasPositions = false;

    AssertingTermVectorsWriter(std::shared_ptr<TermVectorsWriter> in_);

    void startDocument(int numVectorFields)  override;

    void finishDocument()  override;

    void startField(std::shared_ptr<FieldInfo> info, int numTerms,
                    bool positions, bool offsets,
                    bool payloads)  override;

    void finishField()  override;

    void startTerm(std::shared_ptr<BytesRef> term,
                   int freq)  override;

    void finishTerm()  override;

    void
    addPosition(int position, int startOffset, int endOffset,
                std::shared_ptr<BytesRef> payload)  override;

    void finish(std::shared_ptr<FieldInfos> fis,
                int numDocs)  override;

    virtual ~AssertingTermVectorsWriter();

  protected:
    std::shared_ptr<AssertingTermVectorsWriter> shared_from_this()
    {
      return std::static_pointer_cast<AssertingTermVectorsWriter>(
          org.apache.lucene.codecs.TermVectorsWriter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<AssertingTermVectorsFormat> shared_from_this()
  {
    return std::static_pointer_cast<AssertingTermVectorsFormat>(
        org.apache.lucene.codecs.TermVectorsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::asserting
