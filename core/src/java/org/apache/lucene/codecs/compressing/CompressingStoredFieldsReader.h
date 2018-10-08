#pragma once
#include "../../store/DataInput.h"
#include "../../util/IntsRef.h"
#include "../StoredFieldsReader.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class FieldInfos;
}

namespace org::apache::lucene::codecs::compressing
{
class CompressingStoredFieldsIndexReader;
}
namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::codecs::compressing
{
class CompressionMode;
}
namespace org::apache::lucene::codecs::compressing
{
class Decompressor;
}
namespace org::apache::lucene::codecs::compressing
{
class BlockState;
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
namespace org::apache::lucene::store
{
class AlreadyClosedException;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class StoredFieldVisitor;
}
namespace org::apache::lucene::codecs
{
class StoredFieldsReader;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class Accountable;
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.BYTE_ARR;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.CODEC_SFX_DAT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.CODEC_SFX_IDX;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.DAY;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.DAY_ENCODING;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.FIELDS_EXTENSION;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.FIELDS_INDEX_EXTENSION;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.HOUR;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.HOUR_ENCODING;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.NUMERIC_DOUBLE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.NUMERIC_FLOAT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.NUMERIC_INT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.NUMERIC_LONG;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.SECOND;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.SECOND_ENCODING;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.STRING;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.TYPE_BITS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.TYPE_MASK;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.VERSION_CURRENT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.VERSION_START;

using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using DataInput = org::apache::lucene::store::DataInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;

/**
 * {@link StoredFieldsReader} impl for {@link CompressingStoredFieldsFormat}.
 * @lucene.experimental
 */
class CompressingStoredFieldsReader final : public StoredFieldsReader
{
  GET_CLASS_NAME(CompressingStoredFieldsReader)

private:
  const int version;
  const std::shared_ptr<FieldInfos> fieldInfos;
  const std::shared_ptr<CompressingStoredFieldsIndexReader> indexReader;
  const int64_t maxPointer;
  const std::shared_ptr<IndexInput> fieldsStream;
  const int chunkSize;
  const int packedIntsVersion;
  const std::shared_ptr<CompressionMode> compressionMode;
  const std::shared_ptr<Decompressor> decompressor;
  const int numDocs;
  const bool merging;
  const std::shared_ptr<BlockState> state;
  const int64_t numChunks; // number of compressed blocks written
  const int64_t
      numDirtyChunks; // number of incomplete compressed blocks written
  bool closed = false;

  // used by clone
  CompressingStoredFieldsReader(
      std::shared_ptr<CompressingStoredFieldsReader> reader, bool merging);

  /** Sole constructor. */
public:
  CompressingStoredFieldsReader(
      std::shared_ptr<Directory> d, std::shared_ptr<SegmentInfo> si,
      const std::wstring &segmentSuffix, std::shared_ptr<FieldInfos> fn,
      std::shared_ptr<IOContext> context, const std::wstring &formatName,
      std::shared_ptr<CompressionMode> compressionMode) ;

  /**
   * @throws AlreadyClosedException if this FieldsReader is closed
   */
private:
  void ensureOpen() ;

  /**
   * Close the underlying {@link IndexInput}s.
   */
public:
  virtual ~CompressingStoredFieldsReader();

private:
  static void readField(std::shared_ptr<DataInput> in_,
                        std::shared_ptr<StoredFieldVisitor> visitor,
                        std::shared_ptr<FieldInfo> info,
                        int bits) ;

  static void skipField(std::shared_ptr<DataInput> in_,
                        int bits) ;

  /**
   * Reads a float in a variable-length format.  Reads between one and
   * five bytes. Small integral values typically take fewer bytes.
   */
public:
  static float readZFloat(std::shared_ptr<DataInput> in_) ;

  /**
   * Reads a double in a variable-length format.  Reads between one and
   * nine bytes. Small integral values typically take fewer bytes.
   */
  static double readZDouble(std::shared_ptr<DataInput> in_) ;

  /**
   * Reads a long in a variable-length format.  Reads between one andCorePropLo
   * nine bytes. Small values typically take fewer bytes.
   */
  static int64_t readTLong(std::shared_ptr<DataInput> in_) ;

  /**
   * A serialized document, you need to decode its input in order to get an
   * actual
   * {@link Document}.
   */
public:
  class SerializedDocument
      : public std::enable_shared_from_this<SerializedDocument>
  {
    GET_CLASS_NAME(SerializedDocument)

    // the serialized data
  public:
    const std::shared_ptr<DataInput> in_;

    // the number of bytes on which the document is encoded
    const int length;

    // the number of stored fields
    const int numStoredFields;

  private:
    SerializedDocument(std::shared_ptr<DataInput> in_, int length,
                       int numStoredFields);
  };

  /**
   * Keeps state about the current block of documents.
   */
private:
  class BlockState : public std::enable_shared_from_this<BlockState>
  {
    GET_CLASS_NAME(BlockState)
  private:
    std::shared_ptr<CompressingStoredFieldsReader> outerInstance;

  public:
    BlockState(std::shared_ptr<CompressingStoredFieldsReader> outerInstance);

  private:
    int docBase = 0, chunkDocs = 0;

    // whether the block has been sliced, this happens for large documents
    bool sliced = false;

    std::deque<int> offsets = IntsRef::EMPTY_INTS;
    std::deque<int> numStoredFields = IntsRef::EMPTY_INTS;

    // the start pointer at which you can read the compressed documents
    int64_t startPointer = 0;

    const std::shared_ptr<BytesRef> spare = std::make_shared<BytesRef>();
    const std::shared_ptr<BytesRef> bytes = std::make_shared<BytesRef>();

  public:
    virtual bool contains(int docID);

    /**
     * Reset this block so that it stores state for the block
     * that contains the given doc id.
     */
    virtual void reset(int docID) ;

  private:
    void doReset(int docID) ;

    /**
     * Get the serialized representation of the given docID. This docID has
     * to be contained in the current block.
     */
  public:
    virtual std::shared_ptr<SerializedDocument>
    document(int docID) ;

  private:
    class DataInputAnonymousInnerClass : public DataInput
    {
      GET_CLASS_NAME(DataInputAnonymousInnerClass)
    private:
      std::shared_ptr<BlockState> outerInstance;

      int offset = 0;
      int length = 0;

    public:
      DataInputAnonymousInnerClass(std::shared_ptr<BlockState> outerInstance,
                                   int offset, int length);

      int decompressed = 0;

      void fillBuffer() ;

      char readByte()  override;

      void readBytes(std::deque<char> &b, int offset,
                     int len)  override;

    protected:
      std::shared_ptr<DataInputAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DataInputAnonymousInnerClass>(
            org.apache.lucene.store.DataInput::shared_from_this());
      }
    };
  };

public:
  std::shared_ptr<SerializedDocument> document(int docID) ;

  void visitDocument(
      int docID,
      std::shared_ptr<StoredFieldVisitor> visitor)  override;

  std::shared_ptr<StoredFieldsReader> clone() override;

  std::shared_ptr<StoredFieldsReader> getMergeInstance() override;

  int getVersion();

  std::shared_ptr<CompressionMode> getCompressionMode();

  std::shared_ptr<CompressingStoredFieldsIndexReader> getIndexReader();

  int64_t getMaxPointer();

  std::shared_ptr<IndexInput> getFieldsStream();

  int getChunkSize();

  int64_t getNumChunks();

  int64_t getNumDirtyChunks();

  int getPackedIntsVersion();

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  void checkIntegrity()  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<CompressingStoredFieldsReader> shared_from_this()
  {
    return std::static_pointer_cast<CompressingStoredFieldsReader>(
        org.apache.lucene.codecs.StoredFieldsReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::compressing
