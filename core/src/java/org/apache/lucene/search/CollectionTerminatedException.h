#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

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

/** Throw this exception in {@link LeafCollector#collect(int)} to prematurely
 *  terminate collection of the current leaf.
 *  <p>Note: IndexSearcher swallows this exception and never re-throws it.
 *  As a consequence, you should not catch it when calling
 *  {@link IndexSearcher#search} as it is unnecessary and might hide misuse
 *  of this exception. */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("serial") public final class
// CollectionTerminatedException extends RuntimeException
class CollectionTerminatedException final : public std::runtime_error
{

  /** Sole constructor. */
public:
  CollectionTerminatedException();

protected:
  std::shared_ptr<CollectionTerminatedException> shared_from_this()
  {
    return std::static_pointer_cast<CollectionTerminatedException>(
        RuntimeException::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
