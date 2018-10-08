#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
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
namespace org::apache::lucene::expressions::js
{

/**
 * A helper to parse the context of a variable name, which is the base variable,
 * followed by the sequence of array (integer or string indexed) and member
 * accesses.
 */
class VariableContext : public std::enable_shared_from_this<VariableContext>
{
  GET_CLASS_NAME(VariableContext)

  /**
   * Represents what a piece of a variable does.
   */
public:
  enum class Type {
    GET_CLASS_NAME(Type)
    /**
     * A member of the previous context (ie "dot" access).
     */
    MEMBER,

    /**
     * Brackets containing a string as the "index".
     */
    STR_INDEX,

    /**
     * Brackets containing an integer index (ie an array).
     */
    INT_INDEX,

    /**
     * Parenthesis represent a member method to be called.
     */
    METHOD
  };

  /**
   * The type of this piece of a variable.
   */
public:
  const Type type;

  /**
   * The text of this piece of the variable. Used for {@link Type#MEMBER} and
   * {@link Type#STR_INDEX} types.
   */
  const std::wstring text;

  /**
   * The integer value for this piece of the variable. Used for {@link
   * Type#INT_INDEX}.
   */
  const int integer;

private:
  VariableContext(Type c, const std::wstring &s, int i);

  /**
   * Parses a normalized javascript variable. All strings in the variable should
   * be single quoted, and no spaces (except possibly within strings).
   */
public:
  static std::deque<std::shared_ptr<VariableContext>>
  parse(const std::wstring &variable);

  // i points to start of member name
private:
  static int addMember(std::deque<wchar_t> &text, int i,
                       std::deque<std::shared_ptr<VariableContext>> &contexts);

  // i points to start of single quoted index
  static int
  addStringIndex(std::deque<wchar_t> &text, int i,
                 std::deque<std::shared_ptr<VariableContext>> &contexts);

  // i points to start of integer index
  static int
  addIntIndex(std::deque<wchar_t> &text, int i,
              std::deque<std::shared_ptr<VariableContext>> &contexts);
};

} // namespace org::apache::lucene::expressions::js
