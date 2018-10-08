#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/replicator/nrt/PrimaryNode.h"

#include  "core/src/java/org/apache/lucene/index/LeafReader.h"

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

/** A merged segment warmer that pre-copies the merged segment out to
 *  replicas before primary cuts over to the merged segment.  This
 *  ensures that NRT reopen time on replicas is only in proportion to
 *  flushed segment sizes, not merged segments. */

using IndexReaderWarmer =
    org::apache::lucene::index::IndexWriter::IndexReaderWarmer;
using LeafReader = org::apache::lucene::index::LeafReader;

// TODO: or ... replica node can do merging locally?  tricky to keep things in
// sync, when one node merges more slowly than others...

class PreCopyMergedSegmentWarmer
    : public std::enable_shared_from_this<PreCopyMergedSegmentWarmer>,
      public IndexReaderWarmer
{
  GET_CLASS_NAME(PreCopyMergedSegmentWarmer)

private:
  const std::shared_ptr<PrimaryNode> primary;

public:
  PreCopyMergedSegmentWarmer(std::shared_ptr<PrimaryNode> primary);

  void warm(std::shared_ptr<LeafReader> reader)  override;
};

} // #include  "core/src/java/org/apache/lucene/replicator/nrt/
