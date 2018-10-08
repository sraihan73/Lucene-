#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/spatial/ShapeValues.h"

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

namespace org::apache::lucene::spatial
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SegmentCacheable = org::apache::lucene::search::SegmentCacheable;

/**
 * Produces {@link ShapeValues} per-segment
 */
class ShapeValuesSource
    : public std::enable_shared_from_this<ShapeValuesSource>,
      public SegmentCacheable
{
  GET_CLASS_NAME(ShapeValuesSource)

  /**
   * Get a {@link ShapeValues} instance for the given leaf reader context
   */
public:
  virtual std::shared_ptr<ShapeValues>
  getValues(std::shared_ptr<LeafReaderContext> ctx) = 0;
};

} // #include  "core/src/java/org/apache/lucene/spatial/
