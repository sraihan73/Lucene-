#pragma once
#include "../../util/Attribute.h"
#include "stringhelper.h"

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
 * This attribute can be used to mark a token as a keyword. Keyword aware
 * {@link TokenStream}s can decide to modify a token based on the return value
 * of {@link #isKeyword()} if the token is modified. Stemming filters for
 * instance can use this attribute to conditionally skip a term if
 * {@link #isKeyword()} returns <code>true</code>.
 */
class KeywordAttribute : public Attribute
{
  GET_CLASS_NAME(KeywordAttribute)

  /**
   * Returns <code>true</code> if the current token is a keyword, otherwise
   * <code>false</code>
   *
   * @return <code>true</code> if the current token is a keyword, otherwise
   *         <code>false</code>
   * @see #setKeyword(bool)
   */
public:
  virtual bool isKeyword() = 0;

  /**
   * Marks the current token as keyword if set to <code>true</code>.
   *
   * @param isKeyword
   *          <code>true</code> if the current token is a keyword, otherwise
   *          <code>false</code>.
   * @see #isKeyword()
   */
  virtual void setKeyword(bool isKeyword) = 0;
};

} // #include  "core/src/java/org/apache/lucene/analysis/tokenattributes/
