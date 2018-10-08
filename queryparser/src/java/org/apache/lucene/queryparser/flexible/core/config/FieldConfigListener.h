#pragma once
#include "stringhelper.h"
#include <memory>

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
namespace org::apache::lucene::queryparser::flexible::core::config
{

/**
 * This interface should be implemented by classes that wants to listen for
 * field configuration requests. The implementation receives a
 * {@link FieldConfig} object and may add/change its configuration.
 *
 * @see FieldConfig
 * @see QueryConfigHandler
 */
class FieldConfigListener
{
  GET_CLASS_NAME(FieldConfigListener)

  /**
   * This method is called ever time a field configuration is requested.
   *
   * @param fieldConfig
   *          the field configuration requested, should never be null
   */
public:
  virtual void buildFieldConfig(std::shared_ptr<FieldConfig> fieldConfig) = 0;
};

} // namespace org::apache::lucene::queryparser::flexible::core::config
