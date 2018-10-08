#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/query/SpatialArgs.h"

#include  "core/src/java/org/apache/lucene/spatial/query/SpatialArgsParser.h"

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

using org::locationtech::spatial4j::context::SpatialContext;

using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialArgsParser =
    org::apache::lucene::spatial::query::SpatialArgsParser;

/**
 * Helper class to execute queries
 */
class SpatialTestQuery : public std::enable_shared_from_this<SpatialTestQuery>
{
  GET_CLASS_NAME(SpatialTestQuery)
public:
  std::wstring testname;
  std::wstring line;
  int lineNumber = -1;
  std::shared_ptr<SpatialArgs> args;
  std::deque<std::wstring> ids = std::deque<std::wstring>();

  /**
   * Get Test Queries.  The InputStream is closed.
   */
  static std::shared_ptr<Iterator<std::shared_ptr<SpatialTestQuery>>>
  getTestQueries(std::shared_ptr<SpatialArgsParser> parser,
                 std::shared_ptr<SpatialContext> ctx, const std::wstring &name,
                 std::shared_ptr<InputStream> in_) ;

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/spatial/
