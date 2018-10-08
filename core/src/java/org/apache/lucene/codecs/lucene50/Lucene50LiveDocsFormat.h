#pragma once
#include "../LiveDocsFormat.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::util
{
class Bits;
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

using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using SegmentCommitInfo = org::apache::lucene::index::SegmentCommitInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Bits = org::apache::lucene::util::Bits;

/**
 * Lucene 5.0 live docs format
 * <p>The .liv file is optional, and only exists when a segment contains
 * deletions.
 * <p>Although per-segment, this file is maintained exterior to compound segment
 * files.
 * <p>Deletions (.liv) --&gt; IndexHeader,Generation,Bits
 * <ul>
 *   <li>SegmentHeader --&gt; {@link CodecUtil#writeIndexHeader
 * IndexHeader}</li> <li>Bits --&gt; &lt;{@link DataOutput#writeLong Int64}&gt;
 * <sup>LongCount</sup></li>
 * </ul>
 */
class Lucene50LiveDocsFormat final : public LiveDocsFormat
{
  GET_CLASS_NAME(Lucene50LiveDocsFormat)

  /** Sole constructor. */
public:
  Lucene50LiveDocsFormat();

  /** extension of live docs */
private:
  static const std::wstring EXTENSION;

  /** codec of live docs */
  static const std::wstring CODEC_NAME;

  /** supported version range */
  static constexpr int VERSION_START = 0;
  static constexpr int VERSION_CURRENT = VERSION_START;

public:
  std::shared_ptr<Bits>
  readLiveDocs(std::shared_ptr<Directory> dir,
               std::shared_ptr<SegmentCommitInfo> info,
               std::shared_ptr<IOContext> context)  override;

  void
  writeLiveDocs(std::shared_ptr<Bits> bits, std::shared_ptr<Directory> dir,
                std::shared_ptr<SegmentCommitInfo> info, int newDelCount,
                std::shared_ptr<IOContext> context)  override;

  void files(std::shared_ptr<SegmentCommitInfo> info,
             std::shared_ptr<std::deque<std::wstring>> files) 
      override;

protected:
  std::shared_ptr<Lucene50LiveDocsFormat> shared_from_this()
  {
    return std::static_pointer_cast<Lucene50LiveDocsFormat>(
        org.apache.lucene.codecs.LiveDocsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene50
