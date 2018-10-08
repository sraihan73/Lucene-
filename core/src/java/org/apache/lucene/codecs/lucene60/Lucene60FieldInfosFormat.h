#pragma once
#include "../../index/DocValuesType.h"
#include "../../index/IndexOptions.h"
#include "../FieldInfosFormat.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::store
{
class IndexInput;
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
namespace org::apache::lucene::codecs::lucene60
{

using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;

/**
 * Lucene 6.0 Field Infos format.
 * <p>Field names are stored in the field info file, with suffix <tt>.fnm</tt>.
 * <p>FieldInfos (.fnm) --&gt; Header,FieldsCount, &lt;FieldName,FieldNumber,
 * FieldBits,DocValuesBits,DocValuesGen,Attributes,DimensionCount,DimensionNumBytes&gt;
 * <sup>FieldsCount</sup>,Footer <p>Data types: <ul> <li>Header --&gt; {@link
 * CodecUtil#checkIndexHeader IndexHeader}</li> <li>FieldsCount --&gt; {@link
 * DataOutput#writeVInt VInt}</li> <li>FieldName --&gt; {@link
 * DataOutput#writeString std::wstring}</li> <li>FieldBits, IndexOptions,
 * DocValuesBits --&gt; {@link DataOutput#writeByte Byte}</li> <li>FieldNumber,
 * DimensionCount, DimensionNumBytes --&gt; {@link DataOutput#writeInt
 * VInt}</li> <li>Attributes --&gt; {@link DataOutput#writeMapOfStrings
 * Map&lt;std::wstring,std::wstring&gt;}</li> <li>DocValuesGen --&gt; {@link
 * DataOutput#writeLong(long) Int64}</li> <li>Footer --&gt; {@link
 * CodecUtil#writeFooter CodecFooter}</li>
 * </ul>
 * Field Descriptions:
 * <ul>
 *   <li>FieldsCount: the number of fields in this file.</li>
 *   <li>FieldName: name of the field as a UTF-8 std::wstring.</li>
 *   <li>FieldNumber: the field's number. Note that unlike previous versions of
 *       Lucene, the fields are not numbered implicitly by their order in the
 *       file, instead explicitly.</li>
 *   <li>FieldBits: a byte containing field options.
 *     <ul>
 *       <li>The low order bit (0x1) is one for fields that have term vectors
 *           stored, and zero for fields without term vectors.</li>
 *       <li>If the second lowest order-bit is set (0x2), norms are omitted for
 * the indexed field.</li> <li>If the third lowest-order bit is set (0x4),
 * payloads are stored for the indexed field.</li>
 *     </ul>
 *   </li>
 *   <li>IndexOptions: a byte containing index options.
 *     <ul>
 *       <li>0: not indexed</li>
 *       <li>1: indexed as DOCS_ONLY</li>
 *       <li>2: indexed as DOCS_AND_FREQS</li>
 *       <li>3: indexed as DOCS_AND_FREQS_AND_POSITIONS</li>
 *       <li>4: indexed as DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS</li>
 *     </ul>
 *   </li>
 *   <li>DocValuesBits: a byte containing per-document value types. The type
 *       recorded as two four-bit integers, with the high-order bits
 * representing <code>norms</code> options, and the low-order bits representing
 *       {@code DocValues} options. Each four-bit integer can be decoded as
 * such: <ul> <li>0: no DocValues for this field.</li> <li>1: NumericDocValues.
 * ({@link DocValuesType#NUMERIC})</li> <li>2: BinaryDocValues. ({@code
 * DocValuesType#BINARY})</li> <li>3: SortedDocValues. ({@code
 * DocValuesType#SORTED})</li>
 *      </ul>
 *   </li>
 *   <li>DocValuesGen is the generation count of the field's DocValues. If this
 * is -1, there are no DocValues updates to that field. Anything above zero
 * means there are updates stored by {@link DocValuesFormat}.</li>
 *   <li>Attributes: a key-value map_obj of codec-private attributes.</li>
 *   <li>PointDimensionCount, PointNumBytes: these are non-zero only if the
 * field is indexed as points, e.g. using {@link
 * org.apache.lucene.document.LongPoint}</li>
 * </ul>
 *
 * @lucene.experimental
 */
class Lucene60FieldInfosFormat final : public FieldInfosFormat
{
  GET_CLASS_NAME(Lucene60FieldInfosFormat)

  /** Sole constructor. */
public:
  Lucene60FieldInfosFormat();

  std::shared_ptr<FieldInfos>
  read(std::shared_ptr<Directory> directory,
       std::shared_ptr<SegmentInfo> segmentInfo,
       const std::wstring &segmentSuffix,
       std::shared_ptr<IOContext> context)  override;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static Lucene60FieldInfosFormat::StaticConstructor staticConstructor;

  static char docValuesByte(DocValuesType type);

  static DocValuesType getDocValuesType(std::shared_ptr<IndexInput> input,
                                        char b) ;

  static char indexOptionsByte(IndexOptions indexOptions);

  static IndexOptions getIndexOptions(std::shared_ptr<IndexInput> input,
                                      char b) ;

public:
  void write(std::shared_ptr<Directory> directory,
             std::shared_ptr<SegmentInfo> segmentInfo,
             const std::wstring &segmentSuffix,
             std::shared_ptr<FieldInfos> infos,
             std::shared_ptr<IOContext> context)  override;

  /** Extension of field infos */
  static const std::wstring EXTENSION;

  // Codec header
  static const std::wstring CODEC_NAME;
  static constexpr int FORMAT_START = 0;
  static constexpr int FORMAT_SOFT_DELETES = 1;
  static constexpr int FORMAT_CURRENT = FORMAT_SOFT_DELETES;

  // Field flags
  static constexpr char STORE_TERMVECTOR = 0x1;
  static constexpr char OMIT_NORMS = 0x2;
  static constexpr char STORE_PAYLOADS = 0x4;
  static constexpr char SOFT_DELETES_FIELD = 0x8;

protected:
  std::shared_ptr<Lucene60FieldInfosFormat> shared_from_this()
  {
    return std::static_pointer_cast<Lucene60FieldInfosFormat>(
        org.apache.lucene.codecs.FieldInfosFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene60
