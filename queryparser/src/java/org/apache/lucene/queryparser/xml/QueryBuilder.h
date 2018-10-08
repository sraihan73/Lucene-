#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::queryparser::xml
{

using Query = org::apache::lucene::search::Query;
using org::w3c::dom::Element;
/**
 * Implemented by objects that produce Lucene Query objects from XML streams.
 * Implementations are expected to be thread-safe so that they can be used to
 * simultaneously parse multiple XML documents.
 */
class QueryBuilder
{
  GET_CLASS_NAME(QueryBuilder)

public:
  virtual std::shared_ptr<Query> getQuery(std::shared_ptr<Element> e) = 0;
};

} // namespace org::apache::lucene::queryparser::xml
