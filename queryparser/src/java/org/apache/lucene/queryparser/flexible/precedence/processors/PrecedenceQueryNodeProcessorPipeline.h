#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/config/QueryConfigHandler.h"

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
namespace org::apache::lucene::queryparser::flexible::precedence::processors
{

using StandardQueryNodeProcessorPipeline = org::apache::lucene::queryparser::
    flexible::standard::processors::StandardQueryNodeProcessorPipeline;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;

/**
 * <p>
 * This processor pipeline extends {@link StandardQueryNodeProcessorPipeline}
 * and enables bool precedence on it.
 * </p>
 * <p>
 * EXPERT: the precedence is enabled by removing {@link
 * BooleanQuery2ModifierNodeProcessor} from the
 * {@link StandardQueryNodeProcessorPipeline} and appending {@link
 * BooleanModifiersQueryNodeProcessor} to the pipeline.
 * </p>
 *
 * @see PrecedenceQueryParser
 *  @see StandardQueryNodeProcessorPipeline
 */
class PrecedenceQueryNodeProcessorPipeline
    : public StandardQueryNodeProcessorPipeline
{
  GET_CLASS_NAME(PrecedenceQueryNodeProcessorPipeline)

  /**
   * @see
   * StandardQueryNodeProcessorPipeline#StandardQueryNodeProcessorPipeline(QueryConfigHandler)
   */
public:
  PrecedenceQueryNodeProcessorPipeline(
      std::shared_ptr<QueryConfigHandler> queryConfig);

protected:
  std::shared_ptr<PrecedenceQueryNodeProcessorPipeline> shared_from_this()
  {
    return std::static_pointer_cast<PrecedenceQueryNodeProcessorPipeline>(
        org.apache.lucene.queryparser.flexible.standard.processors
            .StandardQueryNodeProcessorPipeline::shared_from_this());
  }
};

} // namespace
  // org::apache::lucene::queryparser::flexible::precedence::processors
