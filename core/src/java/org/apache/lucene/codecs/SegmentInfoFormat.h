#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

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
namespace org::apache::lucene::codecs
{

using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

/**
 * Expert: Controls the format of the
 * {@link SegmentInfo} (segment metadata file).
 * @see SegmentInfo
 * @lucene.experimental
 */
class SegmentInfoFormat : public std::enable_shared_from_this<SegmentInfoFormat>
{
  GET_CLASS_NAME(SegmentInfoFormat)
  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  SegmentInfoFormat();

  /**
   * Read {@link SegmentInfo} data from a directory.
   * @param directory directory to read from
   * @param segmentName name of the segment to read
   * @param segmentID expected identifier for the segment
   * @return infos instance to be populated with data
   * @throws IOException If an I/O error occurs
   */
public:
  virtual std::shared_ptr<SegmentInfo>
  read(std::shared_ptr<Directory> directory, const std::wstring &segmentName,
       std::deque<char> &segmentID, std::shared_ptr<IOContext> context) = 0;

  /**
   * Write {@link SegmentInfo} data.
   * The codec must add its SegmentInfo filename(s) to {@code info} before doing
   * i/o.
   * @throws IOException If an I/O error occurs
   */
  virtual void write(std::shared_ptr<Directory> dir,
                     std::shared_ptr<SegmentInfo> info,
                     std::shared_ptr<IOContext> ioContext) = 0;
};

} // namespace org::apache::lucene::codecs
