#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/config/QueryConfigHandler.h"

#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/config/FieldConfig.h"

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
namespace org::apache::lucene::queryparser::flexible::standard::config
{

using FieldConfig =
    org::apache::lucene::queryparser::flexible::core::config::FieldConfig;
using FieldConfigListener = org::apache::lucene::queryparser::flexible::core::
    config::FieldConfigListener;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;

/**
 * This listener listens for every field configuration request and assign a
 * {@link ConfigurationKeys#BOOST} to the
 * equivalent {@link FieldConfig} based on a defined map_obj: fieldName -&gt;
 * boostValue stored in
 * {@link ConfigurationKeys#FIELD_BOOST_MAP}.
 *
 * @see ConfigurationKeys#FIELD_BOOST_MAP
 * @see ConfigurationKeys#BOOST
 * @see FieldConfig
 * @see FieldConfigListener
 */
class FieldBoostMapFCListener
    : public std::enable_shared_from_this<FieldBoostMapFCListener>,
      public FieldConfigListener
{
  GET_CLASS_NAME(FieldBoostMapFCListener)

private:
  std::shared_ptr<QueryConfigHandler> config = nullptr;

public:
  FieldBoostMapFCListener(std::shared_ptr<QueryConfigHandler> config);

  void buildFieldConfig(std::shared_ptr<FieldConfig> fieldConfig) override;
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/config/
