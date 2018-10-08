#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

#include  "core/src/java/org/apache/lucene/search/join/BitSetProducer.h"

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
namespace org::apache::lucene::search::join
{

using IndexReader = org::apache::lucene::index::IndexReader;

/** Utility class to check a block join index. */
class CheckJoinIndex : public std::enable_shared_from_this<CheckJoinIndex>
{

private:
  CheckJoinIndex();

  /**
   * Check that the given index is good to use for block joins.
   * @throws IllegalStateException if the index does not have an appropriate
   * structure
   */
public:
  static void
  check(std::shared_ptr<IndexReader> reader,
        std::shared_ptr<BitSetProducer> parentsFilter) ;
};

} // #include  "core/src/java/org/apache/lucene/search/join/
