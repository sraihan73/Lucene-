#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::flexible::core::config
{
class FieldConfig;
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
namespace org::apache::lucene::queryparser::flexible::spans
{

using ConfigurationKey =
    org::apache::lucene::queryparser::flexible::core::config::ConfigurationKey;
using FieldConfig =
    org::apache::lucene::queryparser::flexible::core::config::FieldConfig;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;

/**
 * This query config handler only adds the {@link UniqueFieldAttribute} to
 * it.<br> <br>
 *
 * It does not return any configuration for a field in specific.
 */
class SpansQueryConfigHandler : public QueryConfigHandler
{
  GET_CLASS_NAME(SpansQueryConfigHandler)

public:
  static const std::shared_ptr<ConfigurationKey<std::wstring>> UNIQUE_FIELD;

  SpansQueryConfigHandler();

  std::shared_ptr<FieldConfig>
  getFieldConfig(const std::wstring &fieldName) override;

protected:
  std::shared_ptr<SpansQueryConfigHandler> shared_from_this()
  {
    return std::static_pointer_cast<SpansQueryConfigHandler>(
        org.apache.lucene.queryparser.flexible.core.config
            .QueryConfigHandler::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::spans
