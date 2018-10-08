#pragma once
#include "stringhelper.h"
#include <memory>

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
 * Encodes/decodes compound files
 * @lucene.experimental
 */
class CompoundFormat : public std::enable_shared_from_this<CompoundFormat>
{
  GET_CLASS_NAME(CompoundFormat)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
public:
  CompoundFormat();

  // TODO: this is very minimal. If we need more methods,
  // we can add 'producer' classes.

  /**
   * Returns a Directory view (read-only) for the compound files in this segment
   */
  virtual std::shared_ptr<Directory>
  getCompoundReader(std::shared_ptr<Directory> dir,
                    std::shared_ptr<SegmentInfo> si,
                    std::shared_ptr<IOContext> context) = 0;

  /**
   * Packs the provided segment's files into a compound format.  All files
   * referenced by the provided {@link SegmentInfo} must have {@link
   * CodecUtil#writeIndexHeader} and {@link CodecUtil#writeFooter}.
   */
  virtual void write(std::shared_ptr<Directory> dir,
                     std::shared_ptr<SegmentInfo> si,
                     std::shared_ptr<IOContext> context) = 0;
};

} // namespace org::apache::lucene::codecs