#pragma once
#include "../TermVectorsFormat.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::compressing
{
class CompressionMode;
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
namespace org::apache::lucene::codecs::compressing
{

using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

/**
 * A {@link TermVectorsFormat} that compresses chunks of documents together in
 * order to improve the compression ratio.
 * @lucene.experimental
 */
class CompressingTermVectorsFormat : public TermVectorsFormat
{
  GET_CLASS_NAME(CompressingTermVectorsFormat)

private:
  const std::wstring formatName;
  const std::wstring segmentSuffix;
  const std::shared_ptr<CompressionMode> compressionMode;
  const int chunkSize;
  const int blockSize;

  /**
   * Create a new {@link CompressingTermVectorsFormat}.
   * <p>
   * <code>formatName</code> is the name of the format. This name will be used
   * in the file formats to perform
   * {@link CodecUtil#checkIndexHeader codec header checks}.
   * <p>
   * The <code>compressionMode</code> parameter allows you to choose between
   * compression algorithms that have various compression and decompression
   * speeds so that you can pick the one that best fits your indexing and
   * searching throughput. You should never instantiate two
   * {@link CompressingTermVectorsFormat}s that have the same name but
   * different {@link CompressionMode}s.
   * <p>
   * <code>chunkSize</code> is the minimum byte size of a chunk of documents.
   * Higher values of <code>chunkSize</code> should improve the compression
   * ratio but will require more memory at indexing time and might make document
   * loading a little slower (depending on the size of your OS cache compared
   * to the size of your index).
   *
   * @param formatName the name of the {@link StoredFieldsFormat}
   * @param segmentSuffix a suffix to append to files created by this format
   * @param compressionMode the {@link CompressionMode} to use
   * @param chunkSize the minimum number of bytes of a single chunk of stored
   * documents
   * @param blockSize the number of chunks to store in an index block.
   * @see CompressionMode
   */
public:
  CompressingTermVectorsFormat(const std::wstring &formatName,
                               const std::wstring &segmentSuffix,
                               std::shared_ptr<CompressionMode> compressionMode,
                               int chunkSize, int blockSize);

  std::shared_ptr<TermVectorsReader> vectorsReader(
      std::shared_ptr<Directory> directory,
      std::shared_ptr<SegmentInfo> segmentInfo,
      std::shared_ptr<FieldInfos> fieldInfos,
      std::shared_ptr<IOContext> context)  override final;

  std::shared_ptr<TermVectorsWriter> vectorsWriter(
      std::shared_ptr<Directory> directory,
      std::shared_ptr<SegmentInfo> segmentInfo,
      std::shared_ptr<IOContext> context)  override final;

  virtual std::wstring toString();

protected:
  std::shared_ptr<CompressingTermVectorsFormat> shared_from_this()
  {
    return std::static_pointer_cast<CompressingTermVectorsFormat>(
        org.apache.lucene.codecs.TermVectorsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::compressing
