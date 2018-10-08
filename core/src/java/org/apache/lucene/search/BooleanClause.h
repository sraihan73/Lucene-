#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
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

/** A clause in a BooleanQuery. */
class BooleanClause final : public std::enable_shared_from_this<BooleanClause>
{
  GET_CLASS_NAME(BooleanClause)

  /** Specifies how clauses are to occur in matching documents. */
public:
  enum class Occur {
    GET_CLASS_NAME(Occur)

    /** Use this operator for clauses that <i>must</i> appear in the matching
       documents. */
    public :
        // C++ TODO: Most Java annotations will not have direct C++ equivalents:
        // ORIGINAL LINE: MUST { @Override public std::wstring toString() { return
        // "+"; } },
            MUST{std::wstring toString(){return L"+";}
},

    /** Like {@link #MUST} except that these clauses do not participate in
       scoring. */
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: FILTER { @Override public std::wstring toString() { return "#";
    // } },
    FILTER{std::wstring toString(){return L"#";
} // namespace org::apache::lucene::search
}
,

    /** Use this operator for clauses that <i>should</i> appear in the
     * matching documents. For a BooleanQuery with no <code>MUST</code>
     * clauses one or more <code>SHOULD</code> clauses must match a document
     * for the BooleanQuery to match.
     * @see BooleanQuery.Builder#setMinimumNumberShouldMatch
     */
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: SHOULD { @Override public std::wstring toString() { return ""; }
    // },
    SHOULD{std::wstring toString(){return L"";
}
}
,

    /** Use this operator for clauses that <i>must not</i> appear in the
     * matching documents. Note that it is not possible to search for queries
     * that only consist of a <code>MUST_NOT</code> clause. These clauses do not
     * contribute to the score of documents. */
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: MUST_NOT { @Override public std::wstring toString() { return
    // "-"; } };
    MUST_NOT{std::wstring toString(){return L"-";
}
}
;
}
;

/** The query whose matching documents are combined by the bool query.
 */
private:
const std::shared_ptr<Query> query;

const Occur occur;

/** Constructs a BooleanClause.
 */
public:
BooleanClause(std::shared_ptr<Query> query, Occur occur);

Occur getOccur();

std::shared_ptr<Query> getQuery();

bool isProhibited();

bool isRequired();

bool isScoring();

/** Returns true if <code>o</code> is equal to this. */
virtual bool equals(std::any o);

/** Returns a hash code value for this object.*/
virtual int hashCode();

virtual std::wstring toString();
}
;
}
