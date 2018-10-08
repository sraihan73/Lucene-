#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::quality
{
class QualityQuery;
}

namespace org::apache::lucene::search
{
class Query;
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
namespace org::apache::lucene::benchmark::quality
{

using ParseException =
    org::apache::lucene::queryparser::classic::ParseException;
using Query = org::apache::lucene::search::Query;

/**
 * Parse a QualityQuery into a Lucene query.
 */
class QualityQueryParser
{
  GET_CLASS_NAME(QualityQueryParser)

  /**
   * Parse a given QualityQuery into a Lucene query.
   * @param qq the quality query to be parsed.
   * @throws ParseException if parsing failed.
   */
public:
  virtual std::shared_ptr<Query> parse(std::shared_ptr<QualityQuery> qq) = 0;
};

} // namespace org::apache::lucene::benchmark::quality
