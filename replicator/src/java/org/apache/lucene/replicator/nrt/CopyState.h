#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::replicator::nrt
{
class FileMetaData;
}

namespace org::apache::lucene::index
{
class SegmentInfos;
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

namespace org::apache::lucene::replicator::nrt
{

using SegmentInfos = org::apache::lucene::index::SegmentInfos;

/** Holds incRef'd file level details for one point-in-time segment infos on the
 * primary node.
 *
 * @lucene.experimental */
class CopyState : public std::enable_shared_from_this<CopyState>
{
  GET_CLASS_NAME(CopyState)

public:
  const std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>> files;
  const int64_t version;
  const int64_t gen;
  std::deque<char> const infosBytes;
  const std::shared_ptr<Set<std::wstring>> completedMergeFiles;
  const int64_t primaryGen;

  // only non-null on the primary node
  const std::shared_ptr<SegmentInfos> infos;

  CopyState(
      std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>> &files,
      int64_t version, int64_t gen, std::deque<char> &infosBytes,
      std::shared_ptr<Set<std::wstring>> completedMergeFiles,
      int64_t primaryGen, std::shared_ptr<SegmentInfos> infos);

  virtual std::wstring toString();
};

} // namespace org::apache::lucene::replicator::nrt
