#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

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
namespace org::apache::lucene::codecs
{

using SegmentCommitInfo = org::apache::lucene::index::SegmentCommitInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Bits = org::apache::lucene::util::Bits;

/** Format for live/deleted documents
 * @lucene.experimental */
class LiveDocsFormat : public std::enable_shared_from_this<LiveDocsFormat>
{
  GET_CLASS_NAME(LiveDocsFormat)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  LiveDocsFormat();

  /** Read live docs bits. */
public:
  virtual std::shared_ptr<Bits>
  readLiveDocs(std::shared_ptr<Directory> dir,
               std::shared_ptr<SegmentCommitInfo> info,
               std::shared_ptr<IOContext> context) = 0;

  /** Persist live docs bits.  Use {@link
   *  SegmentCommitInfo#getNextDelGen} to determine the
   *  generation of the deletes file you should write to. */
  virtual void writeLiveDocs(std::shared_ptr<Bits> bits,
                             std::shared_ptr<Directory> dir,
                             std::shared_ptr<SegmentCommitInfo> info,
                             int newDelCount,
                             std::shared_ptr<IOContext> context) = 0;

  /** Records all files in use by this {@link SegmentCommitInfo} into the files
   * argument. */
  virtual void files(std::shared_ptr<SegmentCommitInfo> info,
                     std::shared_ptr<std::deque<std::wstring>> files) = 0;
};

} // namespace org::apache::lucene::codecs
