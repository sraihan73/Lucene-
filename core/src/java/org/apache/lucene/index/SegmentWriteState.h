#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class InfoStream;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class SegmentInfo;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class BufferedUpdates;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
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
namespace org::apache::lucene::index
{

using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using InfoStream = org::apache::lucene::util::InfoStream;

/**
 * Holder class for common parameters used during write.
 * @lucene.experimental
 */
class SegmentWriteState : public std::enable_shared_from_this<SegmentWriteState>
{
  GET_CLASS_NAME(SegmentWriteState)

  /** {@link InfoStream} used for debugging messages. */
public:
  const std::shared_ptr<InfoStream> infoStream;

  /** {@link Directory} where this segment will be written
   *  to. */
  const std::shared_ptr<Directory> directory;

  /** {@link SegmentInfo} describing this segment. */
  const std::shared_ptr<SegmentInfo> segmentInfo;

  /** {@link FieldInfos} describing all fields in this
   *  segment. */
  const std::shared_ptr<FieldInfos> fieldInfos;

  /** Number of deleted documents set while flushing the
   *  segment. */
  int delCountOnFlush = 0;
  /** Number of only soft deleted documents set while flushing the
   *  segment. */
  int softDelCountOnFlush = 0;
  /**
   * Deletes and updates to apply while we are flushing the segment. A Term is
   * enrolled in here if it was deleted/updated at one point, and it's mapped to
   * the docIDUpto, meaning any docID &lt; docIDUpto containing this term should
   * be deleted/updated.
   */
  const std::shared_ptr<BufferedUpdates> segUpdates;

  /** {@link FixedBitSet} recording live documents; this is
   *  only set if there is one or more deleted documents. */
  std::shared_ptr<FixedBitSet> liveDocs;

  /** Unique suffix for any postings files written for this
   *  segment.  {@link PerFieldPostingsFormat} sets this for
   *  each of the postings formats it wraps.  If you create
   *  a new {@link PostingsFormat} then any files you
   *  write/read must be derived using this suffix (use
   *  {@link IndexFileNames#segmentFileName(std::wstring,std::wstring,std::wstring)}).
   *
   *  Note: the suffix must be either empty, or be a textual suffix contain
   * exactly two parts (separated by underscore), or be a base36 generation. */
  const std::wstring segmentSuffix;

  /** {@link IOContext} for all writes; you should pass this
   *  to {@link Directory#createOutput(std::wstring,IOContext)}. */
  const std::shared_ptr<IOContext> context;

  /** Sole constructor. */
  SegmentWriteState(std::shared_ptr<InfoStream> infoStream,
                    std::shared_ptr<Directory> directory,
                    std::shared_ptr<SegmentInfo> segmentInfo,
                    std::shared_ptr<FieldInfos> fieldInfos,
                    std::shared_ptr<BufferedUpdates> segUpdates,
                    std::shared_ptr<IOContext> context);

  /**
   * Constructor which takes segment suffix.
   *
   * @see #SegmentWriteState(InfoStream, Directory, SegmentInfo, FieldInfos,
   *      BufferedUpdates, IOContext)
   */
  SegmentWriteState(std::shared_ptr<InfoStream> infoStream,
                    std::shared_ptr<Directory> directory,
                    std::shared_ptr<SegmentInfo> segmentInfo,
                    std::shared_ptr<FieldInfos> fieldInfos,
                    std::shared_ptr<BufferedUpdates> segUpdates,
                    std::shared_ptr<IOContext> context,
                    const std::wstring &segmentSuffix);

  /** Create a shallow copy of {@link SegmentWriteState} with a new segment
   * suffix. */
  SegmentWriteState(std::shared_ptr<SegmentWriteState> state,
                    const std::wstring &segmentSuffix);

  // currently only used by assert? clean up and make real check?
  // either it's a segment suffix (_X_Y) or it's a parseable generation
  // TODO: this is very confusing how ReadersAndUpdates passes generations via
  // this mechanism, maybe add 'generation' explicitly to ctor create the
  // 'actual suffix' here?
private:
  bool assertSegmentSuffix(const std::wstring &segmentSuffix);
};

} // namespace org::apache::lucene::index
