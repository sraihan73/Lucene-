#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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
 * This class represents a field configuration.
 */
class FieldConfig : public AbstractQueryConfig
{
  GET_CLASS_NAME(FieldConfig)

private:
  std::wstring fieldName;

  /**
   * Constructs a {@link FieldConfig}
   *
   * @param fieldName the field name, it must not be null
   * @throws IllegalArgumentException if the field name is null
   */
public:
  FieldConfig(const std::wstring &fieldName);

  /**
   * Returns the field name this configuration represents.
   *
   * @return the field name
   */
  virtual std::wstring getField();

  virtual std::wstring toString();

protected:
  std::shared_ptr<FieldConfig> shared_from_this()
  {
    return std::static_pointer_cast<FieldConfig>(
        AbstractQueryConfig::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::core::config
