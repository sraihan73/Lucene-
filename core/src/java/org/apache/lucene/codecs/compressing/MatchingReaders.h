#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class MergeState;
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
namespace org::apache::lucene::codecs::compressing
{

using MergeState = org::apache::lucene::index::MergeState;

/**
 * Computes which segments have identical field name to number mappings,
 * which allows stored fields and term vectors in this codec to be bulk-merged.
 */
class MatchingReaders : public std::enable_shared_from_this<MatchingReaders>
{
  GET_CLASS_NAME(MatchingReaders)

  /** {@link SegmentReader}s that have identical field
   * name/number mapping, so their stored fields and term
   * vectors may be bulk merged. */
public:
  std::deque<bool> const matchingReaders;

  /** How many {@link #matchingReaders} are set. */
  const int count;

  MatchingReaders(std::shared_ptr<MergeState> mergeState);
};

} // namespace org::apache::lucene::codecs::compressing
