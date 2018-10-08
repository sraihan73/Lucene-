#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::codecs
{
class Codec;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::index
{
class MergeState;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class Builder;
}
namespace org::apache::lucene::index
{
class CodecReader;
}
namespace org::apache::lucene::index
{
class FieldNumbers;
}
namespace org::apache::lucene::index
{
class SegmentInfo;
}
namespace org::apache::lucene::util
{
class InfoStream;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
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

using Codec = org::apache::lucene::codecs::Codec;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using InfoStream = org::apache::lucene::util::InfoStream;

/**
 * The SegmentMerger class combines two or more Segments, represented by an
 * IndexReader, into a single Segment.  Call the merge method to combine the
 * segments.
 *
 * @see #merge
 */
class SegmentMerger final : public std::enable_shared_from_this<SegmentMerger>
{
  GET_CLASS_NAME(SegmentMerger)
private:
  const std::shared_ptr<Directory> directory;

  const std::shared_ptr<Codec> codec;

  const std::shared_ptr<IOContext> context;

public:
  const std::shared_ptr<MergeState> mergeState;

private:
  const std::shared_ptr<FieldInfos::Builder> fieldInfosBuilder;

  // note, just like in codec apis Directory 'dir' is NOT the same as
  // segmentInfo.dir!!
public:
  SegmentMerger(std::deque<std::shared_ptr<CodecReader>> &readers,
                std::shared_ptr<SegmentInfo> segmentInfo,
                std::shared_ptr<InfoStream> infoStream,
                std::shared_ptr<Directory> dir,
                std::shared_ptr<FieldInfos::FieldNumbers> fieldNumbers,
                std::shared_ptr<IOContext> context) ;

  /** True if any merging should happen */
  bool shouldMerge();

  /**
   * Merges the readers into the directory passed to the constructor
   * @return The number of documents that were merged
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  std::shared_ptr<MergeState> merge() ;

private:
  void mergeDocValues(
      std::shared_ptr<SegmentWriteState> segmentWriteState) ;

  void mergePoints(std::shared_ptr<SegmentWriteState> segmentWriteState) throw(
      IOException);

  void mergeNorms(std::shared_ptr<SegmentWriteState> segmentWriteState) throw(
      IOException);

public:
  void mergeFieldInfos() ;

  /**
   * Merge stored fields from each of the segments into the new one.
   * @return The number of documents in all of the readers
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
private:
  int mergeFields() ;

  /**
   * Merge the TermVectors from each of the segments into the new one.
   * @throws IOException if there is a low-level IO error
   */
  int mergeVectors() ;

  void mergeTerms(std::shared_ptr<SegmentWriteState> segmentWriteState) throw(
      IOException);
};

} // namespace org::apache::lucene::index
