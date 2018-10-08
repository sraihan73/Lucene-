#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

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
namespace org::apache::lucene::dependencies
{

/**
 * Parse a properties file, performing recursive Ant-like
 * property value interpolation, and return the resulting Properties.
 */
class InterpolatedProperties : public Properties
{
  GET_CLASS_NAME(InterpolatedProperties)
private:
  static const std::shared_ptr<Pattern> PROPERTY_REFERENCE_PATTERN;

  /**
   * Loads the properties file via {@link Properties#load(InputStream)},
   * then performs recursive Ant-like property value interpolation.
   */
public:
  void load(std::shared_ptr<InputStream> inStream)  override;

  /**
   * Loads the properties file via {@link Properties#load(Reader)},
   * then performs recursive Ant-like property value interpolation.
   */
  void load(std::shared_ptr<Reader> reader)  override;

private:
  static std::unordered_map<std::wstring, std::wstring>
  resolve(std::unordered_map<std::wstring, std::wstring> &props);

  static std::wstring
  resolve(std::unordered_map<std::wstring, std::wstring> &props,
          std::shared_ptr<LinkedHashMap<std::wstring, std::wstring>> resolved,
          std::shared_ptr<Set<std::wstring>> recursive, const std::wstring &key,
          const std::wstring &value);

  static void main(std::deque<std::wstring> &args);

  static void
  assertEquals(std::unordered_map<std::wstring, std::wstring> &resolved,
               std::deque<std::wstring> &keyValuePairs);

protected:
  std::shared_ptr<InterpolatedProperties> shared_from_this()
  {
    return std::static_pointer_cast<InterpolatedProperties>(
        java.util.Properties::shared_from_this());
  }
};

} // namespace org::apache::lucene::dependencies
