#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

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
class MergeState;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::store
{
class DataInput;
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

using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using MergeState = org::apache::lucene::index::MergeState;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using DataInput = org::apache::lucene::store::DataInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using BytesRef = org::apache::lucene::util::BytesRef;

class CrankyTermVectorsFormat : public TermVectorsFormat
{
  GET_CLASS_NAME(CrankyTermVectorsFormat)
public:
  const std::shared_ptr<TermVectorsFormat> delegate_;
  const std::shared_ptr<Random> random;

  CrankyTermVectorsFormat(std::shared_ptr<TermVectorsFormat> delegate_,
                          std::shared_ptr<Random> random);

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
  class CrankyTermVectorsWriter : public TermVectorsWriter
  {
    GET_CLASS_NAME(CrankyTermVectorsWriter)
  public:
    const std::shared_ptr<TermVectorsWriter> delegate_;
    const std::shared_ptr<Random> random;

    CrankyTermVectorsWriter(std::shared_ptr<TermVectorsWriter> delegate_,
                            std::shared_ptr<Random> random);

    int
    merge(std::shared_ptr<MergeState> mergeState)  override;

    void finish(std::shared_ptr<FieldInfos> fis,
                int numDocs)  override;

    virtual ~CrankyTermVectorsWriter();

    // per doc/field methods: lower probability since they are invoked so many
    // times.

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

    void
    addProx(int numProx, std::shared_ptr<DataInput> positions,
            std::shared_ptr<DataInput> offsets)  override;

  protected:
    std::shared_ptr<CrankyTermVectorsWriter> shared_from_this()
    {
      return std::static_pointer_cast<CrankyTermVectorsWriter>(
          org.apache.lucene.codecs.TermVectorsWriter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<CrankyTermVectorsFormat> shared_from_this()
  {
    return std::static_pointer_cast<CrankyTermVectorsFormat>(
        org.apache.lucene.codecs.TermVectorsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::cranky
