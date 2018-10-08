#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Term.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"

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

using Term = org::apache::lucene::index::Term;
using BytesRef = org::apache::lucene::util::BytesRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;

/** A Query that matches documents containing terms with a specified prefix. A
 * PrefixQuery is built by QueryParser for input like <code>app*</code>.
 *
 * <p>This query uses the {@link
 * MultiTermQuery#CONSTANT_SCORE_REWRITE}
 * rewrite method. */

class PrefixQuery : public AutomatonQuery
{
  GET_CLASS_NAME(PrefixQuery)

  /** Constructs a query for terms starting with <code>prefix</code>. */
public:
  PrefixQuery(std::shared_ptr<Term> prefix);

  /** Build an automaton accepting all terms with the specified prefix. */
  static std::shared_ptr<Automaton>
  toAutomaton(std::shared_ptr<BytesRef> prefix);

  /** Returns the prefix of this query. */
  virtual std::shared_ptr<Term> getPrefix();

  /** Prints a user-readable version of this query. */
  std::wstring toString(const std::wstring &field) override;

  virtual int hashCode();

  bool equals(std::any obj) override;

protected:
  std::shared_ptr<PrefixQuery> shared_from_this()
  {
    return std::static_pointer_cast<PrefixQuery>(
        AutomatonQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
