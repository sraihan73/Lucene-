#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::queryparser::xml
{
class ParserException;
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
namespace org::apache::lucene::queryparser::xml::builders
{

using Query = org::apache::lucene::search::Query;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using org::w3c::dom::Element;

/**
 * Creates a range query across 1D {@link PointValues}. The table below
 * specifies the required attributes and the defaults if optional attributes are
 * omitted: <table summary="supported attributes"> <tr> <th>Attribute name</th>
 * <th>Values</th>
 * <th>Required</th>
 * <th>Default</th>
 * </tr>
 * <tr>
 * <td>fieldName</td>
 * <td>std::wstring</td>
 * <td>Yes</td>
 * <td>N/A</td>
 * </tr>
 * <tr>
 * <td>lowerTerm</td>
 * <td>Specified by <tt>type</tt></td>
 * <td>No</td>
 * <td>Integer.MIN_VALUE Long.MIN_VALUE Float.NEGATIVE_INFINITY
 * Double.NEGATIVE_INFINITY</td>
 * </tr>
 * <tr>
 * <td>upperTerm</td>
 * <td>Specified by <tt>type</tt></td>
 * <td>No</td>
 * <td>Integer.MAX_VALUE Long.MAX_VALUE Float.POSITIVE_INFINITY
 * Double.POSITIVE_INFINITY</td>
 * </tr>
 * <tr>
 * <td>type</td>
 * <td>int, long, float, double</td>
 * <td>No</td>
 * <td>int</td>
 * </tr>
 * </table>
 * <p>
 * A {@link ParserException} will be thrown if an error occurs parsing the
 * supplied <tt>lowerTerm</tt> or <tt>upperTerm</tt> into the numeric type
 * specified by <tt>type</tt>.
 */
class PointRangeQueryBuilder
    : public std::enable_shared_from_this<PointRangeQueryBuilder>,
      public QueryBuilder
{
  GET_CLASS_NAME(PointRangeQueryBuilder)

public:
  std::shared_ptr<Query>
  getQuery(std::shared_ptr<Element> e)  override;
};

} // namespace org::apache::lucene::queryparser::xml::builders
