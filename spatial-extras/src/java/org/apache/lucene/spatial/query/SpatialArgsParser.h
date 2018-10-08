#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::query
{
class SpatialArgs;
}

namespace org::apache::lucene::spatial::query
{
class SpatialOperation;
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
namespace org::apache::lucene::spatial::query
{

using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::exception::InvalidShapeException;
using org::locationtech::spatial4j::shape::Shape;

/**
 * Parses a string that usually looks like "OPERATION(SHAPE)" into a {@link
 * SpatialArgs} object. The set of operations supported are defined in {@link
 * SpatialOperation}, such as "Intersects" being a common one. The shape portion
 * is defined by WKT {@link org.locationtech.spatial4j.io.WktShapeParser}, but
 * it can be overridden/customized via {@link #parseShape(std::wstring,
 * org.locationtech.spatial4j.context.SpatialContext)}. There are some optional
 * name-value pair parameters that follow the closing parenthesis.  Example:
 * <pre>
 *   Intersects(ENVELOPE(-10,-8,22,20)) distErrPct=0.025
 * </pre>
 * <p>
 * In the future it would be good to support something at least
 * semi-standardized like a variant of <a
 * href="http://docs.geoserver.org/latest/en/user/filter/ecql_reference.html#spatial-predicate">
 *   [E]CQL</a>.
 *
 * @lucene.experimental
 */
class SpatialArgsParser : public std::enable_shared_from_this<SpatialArgsParser>
{
  GET_CLASS_NAME(SpatialArgsParser)

public:
  static const std::wstring DIST_ERR_PCT;
  static const std::wstring DIST_ERR;

  /** Writes a close approximation to the parsed input format. */
  static std::wstring writeSpatialArgs(std::shared_ptr<SpatialArgs> args);

  /**
   * Parses a string such as "Intersects(ENVELOPE(-10,-8,22,20))
   * distErrPct=0.025".
   *
   * @param v   The string to parse. Mandatory.
   * @param ctx The spatial context. Mandatory.
   * @return Not null.
   * @throws IllegalArgumentException if the parameters don't make sense or an
   * add-on parameter is unknown
   * @throws ParseException If there is a problem parsing the string
   * @throws InvalidShapeException When the coordinates are invalid for the
   * shape
   */
  virtual std::shared_ptr<SpatialArgs>
  parse(const std::wstring &v,
        std::shared_ptr<SpatialContext> ctx) throw(ParseException,
                                                   InvalidShapeException);

protected:
  virtual std::shared_ptr<SpatialArgs>
  newSpatialArgs(std::shared_ptr<SpatialOperation> op,
                 std::shared_ptr<Shape> shape);

  virtual void readNameValuePairs(
      std::shared_ptr<SpatialArgs> args,
      std::unordered_map<std::wstring, std::wstring> &nameValPairs);

  virtual std::shared_ptr<Shape>
  parseShape(const std::wstring &str,
             std::shared_ptr<SpatialContext> ctx) ;

  static std::optional<double> readDouble(const std::wstring &v);

  static bool readBool(const std::wstring &v, bool defaultValue);

  /** Parses "a=b c=d f" (whitespace separated) into name-value pairs. If there
   * is no '=' as in 'f' above then it's short for f=f. */
  static std::unordered_map<std::wstring, std::wstring>
  parseMap(const std::wstring &body);
};

} // namespace org::apache::lucene::spatial::query
