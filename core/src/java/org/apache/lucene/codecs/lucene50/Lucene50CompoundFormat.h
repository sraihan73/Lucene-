#pragma once
#include "../CompoundFormat.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
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

using CompoundFormat = org::apache::lucene::codecs::CompoundFormat;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

/**
 * Lucene 5.0 compound file format
 * <p>
 * Files:
 * <ul>
 *    <li><tt>.cfs</tt>: An optional "virtual" file consisting of all the other
 *    index files for systems that frequently run out of file handles.
 *    <li><tt>.cfe</tt>: The "virtual" compound file's entry table holding all
 *    entries in the corresponding .cfs file.
 * </ul>
 * <p>Description:</p>
 * <ul>
 *   <li>Compound (.cfs) --&gt; Header, FileData <sup>FileCount</sup>,
 * Footer</li> <li>Compound Entry Table (.cfe) --&gt; Header, FileCount,
 * &lt;FileName, DataOffset, DataLength&gt; <sup>FileCount</sup></li> <li>Header
 * --&gt; {@link CodecUtil#writeIndexHeader IndexHeader}</li> <li>FileCount
 * --&gt; {@link DataOutput#writeVInt VInt}</li>
 *   <li>DataOffset,DataLength,Checksum --&gt; {@link DataOutput#writeLong
 * UInt64}</li> <li>FileName --&gt; {@link DataOutput#writeString std::wstring}</li>
 *   <li>FileData --&gt; raw file data</li>
 *   <li>Footer --&gt; {@link CodecUtil#writeFooter CodecFooter}</li>
 * </ul>
 * <p>Notes:</p>
 * <ul>
 *   <li>FileCount indicates how many files are contained in this compound file.
 *       The entry table that follows has that many entries.
 *   <li>Each directory entry contains a long pointer to the start of this
 * file's data section, the files length, and a std::wstring with that file's name.
 * </ul>
 */
class Lucene50CompoundFormat final : public CompoundFormat
{
  GET_CLASS_NAME(Lucene50CompoundFormat)

  /** Sole constructor. */
public:
  Lucene50CompoundFormat();

  std::shared_ptr<Directory> getCompoundReader(
      std::shared_ptr<Directory> dir, std::shared_ptr<SegmentInfo> si,
      std::shared_ptr<IOContext> context)  override;

  void write(std::shared_ptr<Directory> dir, std::shared_ptr<SegmentInfo> si,
             std::shared_ptr<IOContext> context)  override;

  /** Extension of compound file */
  static const std::wstring DATA_EXTENSION;
  /** Extension of compound file entries */
  static const std::wstring ENTRIES_EXTENSION;
  static const std::wstring DATA_CODEC;
  static const std::wstring ENTRY_CODEC;
  static constexpr int VERSION_START = 0;
  static constexpr int VERSION_CURRENT = VERSION_START;

protected:
  std::shared_ptr<Lucene50CompoundFormat> shared_from_this()
  {
    return std::static_pointer_cast<Lucene50CompoundFormat>(
        org.apache.lucene.codecs.CompoundFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene50
