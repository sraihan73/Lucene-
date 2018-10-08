#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

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
namespace org::apache::lucene::queryparser::flexible::precedence
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using StandardQueryParser =
    org::apache::lucene::queryparser::flexible::standard::StandardQueryParser;

/**
 * <p>
 * This query parser works exactly as the standard query parser ( {@link
StandardQueryParser} ),
 * except that it respect the bool precedence, so &lt;a AND b OR c AND d&gt;
is parsed to &lt;(+a +b) (+c +d)&gt;
 * instead of &lt;+a +b +c +d&gt;.
 * </p>
 * <p>
 * EXPERT: This class extends {@link StandardQueryParser}, but uses {@link
PrecedenceQueryNodeProcessorPipeline}
 * instead of {@link StandardQueryNodeProcessorPipeline} to process the query
tree. GET_CLASS_NAME(extends)
 * </p>
 *
 * @see StandardQueryParser
 */
class PrecedenceQueryParser : public StandardQueryParser
{
  GET_CLASS_NAME(PrecedenceQueryParser)

  /**
   * @see StandardQueryParser#StandardQueryParser()
   */
public:
  PrecedenceQueryParser();

  /**
   * @see StandardQueryParser#StandardQueryParser(Analyzer)
   */
  PrecedenceQueryParser(std::shared_ptr<Analyzer> analyer);

protected:
  std::shared_ptr<PrecedenceQueryParser> shared_from_this()
  {
    return std::static_pointer_cast<PrecedenceQueryParser>(
        org.apache.lucene.queryparser.flexible.standard
            .StandardQueryParser::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/precedence/
