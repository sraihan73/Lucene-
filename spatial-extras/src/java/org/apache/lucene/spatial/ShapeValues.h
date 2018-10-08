#pragma once
#include "stringhelper.h"
#include <memory>

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

using org::locationtech::spatial4j::shape::Shape;

/**
 * Iterator over {@link Shape} objects for an index segment
 */
class ShapeValues : public std::enable_shared_from_this<ShapeValues>
{
  GET_CLASS_NAME(ShapeValues)

  /**
   * Advance the iterator to the given document
   * @param doc the document to advance to
   * @return {@code true} if there is a value for this document
   */
public:
  virtual bool advanceExact(int doc) = 0;

  /**
   * Returns a {@link Shape} for the current document
   */
  virtual std::shared_ptr<Shape> value() = 0;
};

} // #include  "core/src/java/org/apache/lucene/spatial/
