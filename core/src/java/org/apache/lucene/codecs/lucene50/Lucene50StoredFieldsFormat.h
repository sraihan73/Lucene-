#pragma once
#include "../StoredFieldsFormat.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::codecs
{
class StoredFieldsFormat;
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
namespace org::apache::lucene::codecs::lucene50
{

using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

/**
 * Lucene 5.0 stored fields format.
 *
 * <p><b>Principle</b>
 * <p>This {@link StoredFieldsFormat} compresses blocks of documents in
 * order to improve the compression ratio compared to document-level
 * compression. It uses the <a href="http://code.google.com/p/lz4/">LZ4</a>
 * compression algorithm by default in 16KB blocks, which is fast to compress
 * and very fast to decompress data. Although the default compression method
 * that is used ({@link Mode#BEST_SPEED BEST_SPEED}) focuses more on speed than
 * on compression ratio, it should provide interesting compression ratios for
 * redundant inputs (such as log files, HTML or plain text). For higher
 * compression, you can choose ({@link Mode#BEST_COMPRESSION BEST_COMPRESSION}),
 * which uses the <a href="http://en.wikipedia.org/wiki/DEFLATE">DEFLATE</a>
 * algorithm with 60KB blocks for a better ratio at the expense of slower
 * performance. These two options can be configured like this: <pre
 * class="prettyprint">
 *   // the default: for high performance
 *   indexWriterConfig.setCodec(new Lucene54Codec(Mode.BEST_SPEED));
 *   // instead for higher performance (but slower):
 *   // indexWriterConfig.setCodec(new Lucene54Codec(Mode.BEST_COMPRESSION));
 * </pre>
 * <p><b>File formats</b>
 * <p>Stored fields are represented by two files:
 * <ol>
 * <li><a name="field_data"></a>
 * <p>A fields data file (extension <tt>.fdt</tt>). This file stores a compact
 * representation of documents in compressed blocks of 16KB or more. When
 * writing a segment, documents are appended to an in-memory <tt>byte[]</tt>
 * buffer. When its size reaches 16KB or more, some metadata about the documents
 * is flushed to disk, immediately followed by a compressed representation of
 * the buffer using the
 * <a href="http://code.google.com/p/lz4/">LZ4</a>
 * <a
 * href="http://fastcompression.blogspot.fr/2011/05/lz4-explained.html">compression
 * format</a>.</p> <p>Here is a more detailed description of the field data file
 * format:</p> <ul> <li>FieldData (.fdt) --&gt; &lt;Header&gt;,
 * PackedIntsVersion, &lt;Chunk&gt;<sup>ChunkCount</sup>, ChunkCount,
 * DirtyChunkCount, Footer</li> <li>Header --&gt; {@link
 * CodecUtil#writeIndexHeader IndexHeader}</li> <li>PackedIntsVersion --&gt;
 * {@link PackedInts#VERSION_CURRENT} as a {@link DataOutput#writeVInt
 * VInt}</li> <li>ChunkCount is not known in advance and is the number of chunks
 * necessary to store all document of the segment</li> <li>Chunk --&gt; DocBase,
 * ChunkDocs, DocFieldCounts, DocLengths, &lt;CompressedDocs&gt;</li>
 * <li>DocBase --&gt; the ID of the first document of the chunk as a {@link
 * DataOutput#writeVInt VInt}</li> <li>ChunkDocs --&gt; the number of documents
 * in the chunk as a {@link DataOutput#writeVInt VInt}</li> <li>DocFieldCounts
 * --&gt; the number of stored fields of every document in the chunk, encoded as
 * followed:<ul> <li>if chunkDocs=1, the unique value is encoded as a {@link
 * DataOutput#writeVInt VInt}</li> <li>else read a {@link DataOutput#writeVInt
 * VInt} (let's call it <tt>bitsRequired</tt>)<ul> <li>if <tt>bitsRequired</tt>
 * is <tt>0</tt> then all values are equal, and the common value is the
 * following {@link DataOutput#writeVInt VInt}</li> <li>else
 * <tt>bitsRequired</tt> is the number of bits required to store any value, and
 * values are stored in a {@link PackedInts packed} array where every value is
 * stored on exactly <tt>bitsRequired</tt> bits</li>
 *   </ul></li>
 * </ul></li>
 * <li>DocLengths --&gt; the lengths of all documents in the chunk, encoded with
 * the same method as DocFieldCounts</li> <li>CompressedDocs --&gt; a compressed
 * representation of &lt;Docs&gt; using the LZ4 compression format</li> <li>Docs
 * --&gt; &lt;Doc&gt;<sup>ChunkDocs</sup></li> <li>Doc --&gt;
 * &lt;FieldNumAndType, Value&gt;<sup>DocFieldCount</sup></li>
 * <li>FieldNumAndType --&gt; a {@link DataOutput#writeVLong VLong}, whose 3
 * last bits are Type and other bits are FieldNum</li> <li>Type --&gt;<ul>
 *   <li>0: Value is std::wstring</li>
 *   <li>1: Value is BinaryValue</li>
 *   <li>2: Value is Int</li>
 *   <li>3: Value is Float</li>
 *   <li>4: Value is Long</li>
 *   <li>5: Value is Double</li>
 *   <li>6, 7: unused</li>
 * </ul></li>
 * <li>FieldNum --&gt; an ID of the field</li>
 * <li>Value --&gt; {@link DataOutput#writeString(std::wstring) std::wstring} | BinaryValue
 * | Int | Float | Long | Double depending on Type</li> <li>BinaryValue --&gt;
 * ValueLength &lt;Byte&gt;<sup>ValueLength</sup></li> <li>ChunkCount --&gt; the
 * number of chunks in this file</li> <li>DirtyChunkCount --&gt; the number of
 * prematurely flushed chunks in this file</li> <li>Footer --&gt; {@link
 * CodecUtil#writeFooter CodecFooter}</li>
 * </ul>
 * <p>Notes
 * <ul>
 * <li>If documents are larger than 16KB then chunks will likely contain only
 * one document. However, documents can never spread across several chunks (all
 * fields of a single document are in the same chunk).</li>
 * <li>When at least one document in a chunk is large enough so that the chunk
 * is larger than 32KB, the chunk will actually be compressed in several LZ4
 * blocks of 16KB. This allows {@link StoredFieldVisitor}s which are only
 * interested in the first fields of a document to not have to decompress 10MB
 * of data if the document is 10MB, but only 16KB.</li>
 * <li>Given that the original lengths are written in the metadata of the chunk,
 * the decompressor can leverage this information to stop decoding as soon as
 * enough data has been decompressed.</li>
 * <li>In case documents are incompressible, CompressedDocs will be less than
 * 0.5% larger than Docs.</li>
 * </ul>
 * </li>
 * <li><a name="field_index"></a>
 * <p>A fields index file (extension <tt>.fdx</tt>).</p>
 * <ul>
 * <li>FieldsIndex (.fdx) --&gt; &lt;Header&gt;, &lt;ChunkIndex&gt;, Footer</li>
 * <li>Header --&gt; {@link CodecUtil#writeIndexHeader IndexHeader}</li>
 * <li>ChunkIndex: See {@link CompressingStoredFieldsIndexWriter}</li>
 * <li>Footer --&gt; {@link CodecUtil#writeFooter CodecFooter}</li>
 * </ul>
 * </li>
 * </ol>
 * <p><b>Known limitations</b>
 * <p>This {@link StoredFieldsFormat} does not support individual documents
 * larger than (<tt>2<sup>31</sup> - 2<sup>14</sup></tt>) bytes.
 * @lucene.experimental
 */
class Lucene50StoredFieldsFormat final : public StoredFieldsFormat
{
  GET_CLASS_NAME(Lucene50StoredFieldsFormat)

  /** Configuration option for stored fields. */
public:
  enum class Mode {
    GET_CLASS_NAME(Mode)
    /** Trade compression ratio for retrieval speed. */
    BEST_SPEED,
    /** Trade retrieval speed for compression ratio. */
    BEST_COMPRESSION
  };

  /** Attribute key for compression mode. */
public:
  static const std::wstring MODE_KEY;

  const Mode mode;

  /** Stored fields format with default options */
  Lucene50StoredFieldsFormat();

  /** Stored fields format with specified mode */
  Lucene50StoredFieldsFormat(Mode mode);

  std::shared_ptr<StoredFieldsReader>
  fieldsReader(std::shared_ptr<Directory> directory,
               std::shared_ptr<SegmentInfo> si, std::shared_ptr<FieldInfos> fn,
               std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<StoredFieldsWriter>
  fieldsWriter(std::shared_ptr<Directory> directory,
               std::shared_ptr<SegmentInfo> si,
               std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<StoredFieldsFormat> impl(Mode mode);

protected:
  std::shared_ptr<Lucene50StoredFieldsFormat> shared_from_this()
  {
    return std::static_pointer_cast<Lucene50StoredFieldsFormat>(
        org.apache.lucene.codecs.StoredFieldsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene50
