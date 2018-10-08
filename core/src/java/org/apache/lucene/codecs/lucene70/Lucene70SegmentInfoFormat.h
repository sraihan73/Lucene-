#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"

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
namespace org::apache::lucene::codecs::lucene70
{

using SegmentInfoFormat = org::apache::lucene::codecs::SegmentInfoFormat;
using SegmentInfo = org::apache::lucene::index::SegmentInfo; // javadocs
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

/**
 * Lucene 7.0 Segment info format.
 * <p>
 * Files:
 * <ul>
 *   <li><tt>.si</tt>: Header, SegVersion, SegSize, IsCompoundFile, Diagnostics,
 * Files, Attributes, IndexSort, Footer
 * </ul>
 * Data types:
 * <ul>
 *   <li>Header --&gt; {@link CodecUtil#writeIndexHeader IndexHeader}</li>
 *   <li>SegSize --&gt; {@link DataOutput#writeInt Int32}</li>
 *   <li>SegVersion --&gt; {@link DataOutput#writeString std::wstring}</li>
 *   <li>SegMinVersion --&gt; {@link DataOutput#writeString std::wstring}</li>
 *   <li>Files --&gt; {@link DataOutput#writeSetOfStrings
 * Set&lt;std::wstring&gt;}</li> <li>Diagnostics,Attributes --&gt; {@link
 * DataOutput#writeMapOfStrings Map&lt;std::wstring,std::wstring&gt;}</li>
 *   <li>IsCompoundFile --&gt; {@link DataOutput#writeByte Int8}</li>
 *   <li>IndexSort --&gt; {@link DataOutput#writeVInt Int32} count, followed by
 * {@code count} SortField</li> <li>SortField --&gt; {@link
 * DataOutput#writeString std::wstring} field name, followed by {@link
 * DataOutput#writeVInt Int32} sort type ID, followed by {@link
 * DataOutput#writeByte Int8} indicatating reversed sort, followed by a
 * type-specific encoding of the optional missing value <li>Footer --&gt; {@link
 * CodecUtil#writeFooter CodecFooter}</li>
 * </ul>
 * Field Descriptions:
 * <ul>
 *   <li>SegVersion is the code version that created the segment.</li>
 *   <li>SegMinVersion is the minimum code version that contributed documents to
 * the segment.</li> <li>SegSize is the number of documents contained in the
 * segment index.</li> <li>IsCompoundFile records whether the segment is written
 * as a compound file or not. If this is -1, the segment is not a compound file.
 * If it is 1, the segment is a compound file.</li> <li>The Diagnostics Map is
 * privately written by {@link IndexWriter}, as a debugging aid, for each
 * segment it creates. It includes metadata like the current Lucene version, OS,
 * Java version, why the segment was created (merge, flush, addIndexes),
 * etc.</li> <li>Files is a deque of files referred to by this segment.</li>
 * </ul>
 *
 * @see SegmentInfos
 * @lucene.experimental
 */
class Lucene70SegmentInfoFormat : public SegmentInfoFormat
{
  GET_CLASS_NAME(Lucene70SegmentInfoFormat)

  /** Sole constructor. */
public:
  Lucene70SegmentInfoFormat();

  std::shared_ptr<SegmentInfo>
  read(std::shared_ptr<Directory> dir, const std::wstring &segment,
       std::deque<char> &segmentID,
       std::shared_ptr<IOContext> context)  override;

  void write(std::shared_ptr<Directory> dir, std::shared_ptr<SegmentInfo> si,
             std::shared_ptr<IOContext> ioContext)  override;

  /** File extension used to store {@link SegmentInfo}. */
  static const std::wstring SI_EXTENSION;
  static const std::wstring CODEC_NAME;
  static constexpr int VERSION_START = 0;
  static constexpr int VERSION_CURRENT = VERSION_START;

protected:
  std::shared_ptr<Lucene70SegmentInfoFormat> shared_from_this()
  {
    return std::static_pointer_cast<Lucene70SegmentInfoFormat>(
        org.apache.lucene.codecs.SegmentInfoFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene70/
