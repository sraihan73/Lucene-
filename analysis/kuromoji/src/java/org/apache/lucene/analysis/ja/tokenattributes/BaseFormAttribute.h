#pragma once
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/Attribute.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ja
{
class Token;
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
namespace org::apache::lucene::analysis::ja::tokenattributes
{

using Token = org::apache::lucene::analysis::ja::Token;
using Attribute = org::apache::lucene::util::Attribute;

/**
 * Attribute for {@link Token#getBaseForm()}.
 * <p>
 * Note: depending on part of speech, this value may not be applicable,
 * and will be null.
 */
class BaseFormAttribute : public Attribute
{
  GET_CLASS_NAME(BaseFormAttribute)
public:
  virtual std::wstring getBaseForm() = 0;
  virtual void setToken(std::shared_ptr<Token> token) = 0;
};

} // namespace org::apache::lucene::analysis::ja::tokenattributes
