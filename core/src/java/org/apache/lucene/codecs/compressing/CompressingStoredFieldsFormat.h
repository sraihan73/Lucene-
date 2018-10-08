#pragma once
#include "../StoredFieldsFormat.h"
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
class StoredFieldsReader;
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
class StoredFieldsWriter;
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

using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

/**
 * A {@link StoredFieldsFormat} that compresses documents in chunks in
 * order to improve the compression ratio.
 * <p>
 * For a chunk size of <tt>chunkSize</tt> bytes, this {@link StoredFieldsFormat}
 * does not support documents larger than (<tt>2<sup>31</sup> - chunkSize</tt>)
 * bytes.
 * <p>
 * For optimal performance, you should use a {@link MergePolicy} that returns
 * segments that have the biggest byte size first.
 * @lucene.experimental
 */
class CompressingStoredFieldsFormat : public StoredFieldsFormat
{
  GET_CLASS_NAME(CompressingStoredFieldsFormat)

private:
  const std::wstring formatName;
  const std::wstring segmentSuffix;
  const std::shared_ptr<CompressionMode> compressionMode;
  const int chunkSize;
  const int maxDocsPerChunk;
  const int blockSize;

  /**
   * Create a new {@link CompressingStoredFieldsFormat} with an empty segment
   * suffix.
   *
   * @see CompressingStoredFieldsFormat#CompressingStoredFieldsFormat(std::wstring,
   * std::wstring, CompressionMode, int, int, int)
   */
public:
  CompressingStoredFieldsFormat(
      const std::wstring &formatName,
      std::shared_ptr<CompressionMode> compressionMode, int chunkSize,
      int maxDocsPerChunk, int blockSize);

  /**
   * Create a new {@link CompressingStoredFieldsFormat}.
   * <p>
   * <code>formatName</code> is the name of the format. This name will be used
   * in the file formats to perform
   * {@link CodecUtil#checkIndexHeader codec header checks}.
   * <p>
   * <code>segmentSuffix</code> is the segment suffix. This suffix is added to
   * the result file name only if it's not the empty string.
   * <p>
   * The <code>compressionMode</code> parameter allows you to choose between
   * compression algorithms that have various compression and decompression
   * speeds so that you can pick the one that best fits your indexing and
   * searching throughput. You should never instantiate two
   * {@link CompressingStoredFieldsFormat}s that have the same name but
   * different {@link CompressionMode}s.
   * <p>
   * <code>chunkSize</code> is the minimum byte size of a chunk of documents.
   * A value of <code>1</code> can make sense if there is redundancy across
   * fields.
   * <code>maxDocsPerChunk</code> is an upperbound on how many docs may be
   * stored in a single chunk. This is to bound the cpu costs for highly
   * compressible data. <p> Higher values of <code>chunkSize</code> should
   * improve the compression ratio but will require more memory at indexing time
   * and might make document loading a little slower (depending on the size of
   * your OS cache compared to the size of your index).
   *
   * @param formatName the name of the {@link StoredFieldsFormat}
   * @param compressionMode the {@link CompressionMode} to use
   * @param chunkSize the minimum number of bytes of a single chunk of stored
   * documents
   * @param maxDocsPerChunk the maximum number of documents in a single chunk
   * @param blockSize the number of chunks to store in an index block
   * @see CompressionMode
   */
  CompressingStoredFieldsFormat(
      const std::wstring &formatName, const std::wstring &segmentSuffix,
      std::shared_ptr<CompressionMode> compressionMode, int chunkSize,
      int maxDocsPerChunk, int blockSize);

  std::shared_ptr<StoredFieldsReader>
  fieldsReader(std::shared_ptr<Directory> directory,
               std::shared_ptr<SegmentInfo> si, std::shared_ptr<FieldInfos> fn,
               std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<StoredFieldsWriter>
  fieldsWriter(std::shared_ptr<Directory> directory,
               std::shared_ptr<SegmentInfo> si,
               std::shared_ptr<IOContext> context)  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<CompressingStoredFieldsFormat> shared_from_this()
  {
    return std::static_pointer_cast<CompressingStoredFieldsFormat>(
        org.apache.lucene.codecs.StoredFieldsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::compressing
