#pragma once
#include "../../util/Attribute.h"
#include "stringhelper.h"
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
namespace org::apache::lucene::analysis::tokenattributes
{

using Attribute = org::apache::lucene::util::Attribute;

/**
 * A Token's lexical type. The Default value is "word".
 */
class TypeAttribute : public Attribute
{
  GET_CLASS_NAME(TypeAttribute)

  /** the default type */
public:
  static const std::wstring DEFAULT_TYPE;

  /**
   * Returns this Token's lexical type.  Defaults to "word".
   * @see #setType(std::wstring)
   */
  virtual std::wstring type() = 0;

  /**
   * Set the lexical type.
   * @see #type()
   */
  virtual void setType(const std::wstring &type) = 0;
};

} // namespace org::apache::lucene::analysis::tokenattributes
