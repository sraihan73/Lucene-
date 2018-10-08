#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

#include  "core/src/java/org/apache/lucene/queryparser/xml/ParserException.h"

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

using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using org::w3c::dom::Element;
/**
 * Builder for {@link SpanTermQuery}
 */
class SpanTermBuilder : public SpanBuilderBase
{
  GET_CLASS_NAME(SpanTermBuilder)

public:
  std::shared_ptr<SpanQuery>
  getSpanQuery(std::shared_ptr<Element> e)  override;

protected:
  std::shared_ptr<SpanTermBuilder> shared_from_this()
  {
    return std::static_pointer_cast<SpanTermBuilder>(
        SpanBuilderBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/xml/builders/
