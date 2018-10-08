#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <typeinfo>

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
namespace org::apache::lucene::analysis::util
{

/**
 * Abstraction for loading resources (streams, files, and classes).
 */
class ResourceLoader
{
  GET_CLASS_NAME(ResourceLoader)

  /**
   * Opens a named resource
   */
public:
  virtual std::shared_ptr<InputStream>
  openResource(const std::wstring &resource) = 0;

  /**
   * Finds class of the name and expected type
   */
  template <typename T>
  std::type_info findClass(const std::wstring &cname,
                           std::type_info<T> &expectedType) = 0;

  /**
   * Creates an instance of the name and expected type
   */
  // TODO: fix exception handling
  template <typename T>
  T newInstance(const std::wstring &cname, std::type_info<T> &expectedType) = 0;
};
} // #include  "core/src/java/org/apache/lucene/analysis/util/
