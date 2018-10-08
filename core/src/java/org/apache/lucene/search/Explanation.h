#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
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
namespace org::apache::lucene::search
{

/** Expert: Describes the score computation for document and query. */
class Explanation final : public std::enable_shared_from_this<Explanation>
{
  GET_CLASS_NAME(Explanation)

  /**
   * Create a new explanation for a match.
   * @param value       the contribution to the score of the document
   * @param description how {@code value} was computed
   * @param details     sub explanations that contributed to this explanation
   */
public:
  static std::shared_ptr<Explanation>
  match(float value, const std::wstring &description,
        std::shared_ptr<std::deque<std::shared_ptr<Explanation>>> details);

  /**
   * Create a new explanation for a match.
   * @param value       the contribution to the score of the document
   * @param description how {@code value} was computed
   * @param details     sub explanations that contributed to this explanation
   */
  static std::shared_ptr<Explanation> match(float value,
                                            const std::wstring &description,
                                            std::deque<Explanation> &details);

  /**
   * Create a new explanation for a document which does not match.
   */
  static std::shared_ptr<Explanation>
  noMatch(const std::wstring &description,
          std::shared_ptr<std::deque<std::shared_ptr<Explanation>>> details);

  /**
   * Create a new explanation for a document which does not match.
   */
  static std::shared_ptr<Explanation>
  noMatch(const std::wstring &description, std::deque<Explanation> &details);

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool match_;              // whether the document matched
  const float value;              // the value of this node
  const std::wstring description; // what it represents
  const std::deque<std::shared_ptr<Explanation>> details; // sub-explanations

  /** Create a new explanation  */
  Explanation(
      bool match, float value, const std::wstring &description,
      std::shared_ptr<std::deque<std::shared_ptr<Explanation>>> details);

  /**
   * Indicates whether or not this Explanation models a match.
   */
public:
  bool isMatch();

  /** The value assigned to this explanation node. */
  float getValue();

  /** A description of this explanation node. */
  std::wstring getDescription();

private:
  std::wstring getSummary();

  /** The sub-nodes of this explanation node. */
public:
  std::deque<std::shared_ptr<Explanation>> getDetails();

  /** Render an explanation as text. */
  std::wstring toString();

private:
  std::wstring toString(int depth);

public:
  bool equals(std::any o);

  int hashCode();
};

} // namespace org::apache::lucene::search
