#pragma once
#include "stringhelper.h"
#include <memory>
#include <typeinfo>
#include <unordered_map>

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

namespace org::apache::lucene::spatial3d::geom
{

/**
 * Lookup tables for classes that can be serialized using a code.
 *
 * @lucene.internal
 */
class StandardObjects : public std::enable_shared_from_this<StandardObjects>
{
  GET_CLASS_NAME(StandardObjects)

  /**
   * Registry of standard classes to corresponding code
   */
public:
  static std::unordered_map<std::type_info, int> classRegsitry;
  /**
   * Registry of codes to corresponding classes
   */
  static std::unordered_map<int, std::type_info> codeRegsitry;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static StandardObjects::StaticConstructor staticConstructor;
};

} // namespace org::apache::lucene::spatial3d::geom
