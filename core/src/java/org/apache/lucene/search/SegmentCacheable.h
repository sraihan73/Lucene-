#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

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

namespace org::apache::lucene::search
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

/**
 * Interface defining whether or not an object can be cached against a {@link
 * LeafReader}
 *
 * Objects that depend only on segment-immutable structures such as Points or
 * postings lists can just return {@code true} from {@link
 * #isCacheable(LeafReaderContext)}
 *
 * Objects that depend on doc values should return {@link
 * DocValues#isCacheable(LeafReaderContext, std::wstring...)}, which will check to see
 * if the doc values fields have been updated.  Updated doc values fields are
 * not suitable for cacheing.
 *
 * Objects that are not segment-immutable, such as those that rely on global
 * statistics or scores, should return {@code false}
 */
class SegmentCacheable
{
  GET_CLASS_NAME(SegmentCacheable)

  /**
   * @return {@code true} if the object can be cached against a given leaf
   */
public:
  virtual bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) = 0;
};

} // #include  "core/src/java/org/apache/lucene/search/
