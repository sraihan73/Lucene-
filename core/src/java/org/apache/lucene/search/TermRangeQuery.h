#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::util::automaton
{
class Automaton;
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
namespace org::apache::lucene::search
{

using BytesRef = org::apache::lucene::util::BytesRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;

/**
 * A Query that matches documents within an range of terms.
 *
 * <p>This query matches the documents looking for terms that fall into the
 * supplied range according to {@link BytesRef#compareTo(BytesRef)}.
 *
 * <p>This query uses the {@link
 * MultiTermQuery#CONSTANT_SCORE_REWRITE}
 * rewrite method.
 * @since 2.9
 */

class TermRangeQuery : public AutomatonQuery
{
  GET_CLASS_NAME(TermRangeQuery)
private:
  const std::shared_ptr<BytesRef> lowerTerm;
  const std::shared_ptr<BytesRef> upperTerm;
  const bool includeLower;
  const bool includeUpper;

  /**
   * Constructs a query selecting all terms greater/equal than
   * <code>lowerTerm</code> but less/equal than <code>upperTerm</code>.
   *
   * <p>
   * If an endpoint is null, it is said
   * to be "open". Either or both endpoints may be open.  Open endpoints may not
   * be exclusive (you can't select all but the first or last term without
   * explicitly specifying the term to exclude.)
   *
   * @param field The field that holds both lower and upper terms.
   * @param lowerTerm
   *          The term text at the lower end of the range
   * @param upperTerm
   *          The term text at the upper end of the range
   * @param includeLower
   *          If true, the <code>lowerTerm</code> is
   *          included in the range.
   * @param includeUpper
   *          If true, the <code>upperTerm</code> is
   *          included in the range.
   */
public:
  TermRangeQuery(const std::wstring &field, std::shared_ptr<BytesRef> lowerTerm,
                 std::shared_ptr<BytesRef> upperTerm, bool includeLower,
                 bool includeUpper);

  static std::shared_ptr<Automaton>
  toAutomaton(std::shared_ptr<BytesRef> lowerTerm,
              std::shared_ptr<BytesRef> upperTerm, bool includeLower,
              bool includeUpper);

  /**
   * Factory that creates a new TermRangeQuery using Strings for term text.
   */
  static std::shared_ptr<TermRangeQuery>
  newStringRange(const std::wstring &field, const std::wstring &lowerTerm,
                 const std::wstring &upperTerm, bool includeLower,
                 bool includeUpper);

  /** Returns the lower value of this range query */
  virtual std::shared_ptr<BytesRef> getLowerTerm();

  /** Returns the upper value of this range query */
  virtual std::shared_ptr<BytesRef> getUpperTerm();

  /** Returns <code>true</code> if the lower endpoint is inclusive */
  virtual bool includesLower();

  /** Returns <code>true</code> if the upper endpoint is inclusive */
  virtual bool includesUpper();

  /** Prints a user-readable version of this query. */
  std::wstring toString(const std::wstring &field) override;

  virtual int hashCode();

  bool equals(std::any obj) override;

protected:
  std::shared_ptr<TermRangeQuery> shared_from_this()
  {
    return std::static_pointer_cast<TermRangeQuery>(
        AutomatonQuery::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
