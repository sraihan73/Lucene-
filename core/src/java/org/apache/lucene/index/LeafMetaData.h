#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/Version.h"

#include  "core/src/java/org/apache/lucene/search/Sort.h"

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

using Sort = org::apache::lucene::search::Sort;
using Version = org::apache::lucene::util::Version;

/**
 * Provides read-only metadata about a leaf.
 * @lucene.experimental
 */
class LeafMetaData final : public std::enable_shared_from_this<LeafMetaData>
{
  GET_CLASS_NAME(LeafMetaData)

private:
  const int createdVersionMajor;
  const std::shared_ptr<Version> minVersion;
  const std::shared_ptr<Sort> sort;

  /** Expert: Sole constructor. Public for use by custom {@link LeafReader}
   * impls. */
public:
  LeafMetaData(int createdVersionMajor, std::shared_ptr<Version> minVersion,
               std::shared_ptr<Sort> sort);

  /** Get the Lucene version that created this index. This can be used to
   * implement backward compatibility on top of the codec API. A return value of
   * {@code 6} indicates that the created version is unknown. */
  int getCreatedVersionMajor();

  /**
   * Return the minimum Lucene version that contributed documents to this index,
   * or {@code null} if this information is not available.
   */
  std::shared_ptr<Version> getMinVersion();

  /**
   * Return the order in which documents from this index are sorted, or
   * {@code null} if documents are in no particular order.
   */
  std::shared_ptr<Sort> getSort();
};

} // #include  "core/src/java/org/apache/lucene/index/
