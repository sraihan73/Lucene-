#pragma once
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/Attribute.h"
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
namespace org::apache::lucene::analysis::icu::tokenattributes
{

using Attribute = org::apache::lucene::util::Attribute;

/**
 * This attribute stores the UTR #24 script value for a token of text.
 * @lucene.experimental
 */
class ScriptAttribute : public Attribute
{
  GET_CLASS_NAME(ScriptAttribute)
  /**
   * Get the numeric code for this script value.
   * This is the constant value from {@link UScript}.
   * @return numeric code
   */
public:
  virtual int getCode() = 0;
  /**
   * Set the numeric code for this script value.
   * This is the constant value from {@link UScript}.
   * @param code numeric code
   */
  virtual void setCode(int code) = 0;
  /**
   * Get the full name.
   * @return UTR #24 full name.
   */
  virtual std::wstring getName() = 0;
  /**
   * Get the abbreviated name.
   * @return UTR #24 abbreviated name.
   */
  virtual std::wstring getShortName() = 0;
};

} // #include  "core/src/java/org/apache/lucene/analysis/icu/tokenattributes/
