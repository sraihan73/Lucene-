#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
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

/**
 * Holder class for common parameters used during read.
 * @lucene.experimental
 */
class SegmentReadState : public std::enable_shared_from_this<SegmentReadState>
{
  GET_CLASS_NAME(SegmentReadState)
  /** {@link Directory} where this segment is read from. */
public:
  const std::shared_ptr<Directory> directory;

  /** {@link SegmentInfo} describing this segment. */
  const std::shared_ptr<SegmentInfo> segmentInfo;

  /** {@link FieldInfos} describing all fields in this
   *  segment. */
  const std::shared_ptr<FieldInfos> fieldInfos;

  /** {@link IOContext} to pass to {@link
   *  Directory#openInput(std::wstring,IOContext)}. */
  const std::shared_ptr<IOContext> context;

  /** Unique suffix for any postings files read for this
   *  segment.  {@link PerFieldPostingsFormat} sets this for
   *  each of the postings formats it wraps.  If you create
   *  a new {@link PostingsFormat} then any files you
   *  write/read must be derived using this suffix (use
   *  {@link IndexFileNames#segmentFileName(std::wstring,std::wstring,std::wstring)}). */
  const std::wstring segmentSuffix;

  /** Create a {@code SegmentReadState}. */
  SegmentReadState(std::shared_ptr<Directory> dir,
                   std::shared_ptr<SegmentInfo> info,
                   std::shared_ptr<FieldInfos> fieldInfos,
                   std::shared_ptr<IOContext> context);

  /** Create a {@code SegmentReadState}. */
  SegmentReadState(std::shared_ptr<Directory> dir,
                   std::shared_ptr<SegmentInfo> info,
                   std::shared_ptr<FieldInfos> fieldInfos,
                   std::shared_ptr<IOContext> context,
                   const std::wstring &segmentSuffix);

  /** Create a {@code SegmentReadState}. */
  SegmentReadState(std::shared_ptr<SegmentReadState> other,
                   const std::wstring &newSegmentSuffix);
};

} // namespace org::apache::lucene::index
