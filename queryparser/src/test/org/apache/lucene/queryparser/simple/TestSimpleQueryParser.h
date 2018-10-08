#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

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
namespace org::apache::lucene::queryparser::simple
{

using Query = org::apache::lucene::search::Query;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.AND_OPERATOR;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.ESCAPE_OPERATOR;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.FUZZY_OPERATOR;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.NOT_OPERATOR;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.OR_OPERATOR;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.PHRASE_OPERATOR;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.PRECEDENCE_OPERATORS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.PREFIX_OPERATOR;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.NEAR_OPERATOR;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.queryparser.simple.SimpleQueryParser.WHITESPACE_OPERATOR;

/** Tests for {@link SimpleQueryParser} */
class TestSimpleQueryParser : public LuceneTestCase
{
  GET_CLASS_NAME(TestSimpleQueryParser)

  /**
   * helper to parse a query with whitespace+lowercase analyzer across "field",
   * with default operator of MUST
   */
private:
  std::shared_ptr<Query> parse(const std::wstring &text);

  /**
   * helper to parse a query with whitespace+lowercase analyzer across "field",
   * with default operator of MUST
   */
  std::shared_ptr<Query> parse(const std::wstring &text, int flags);

  /** test a simple term */
public:
  virtual void testTerm() ;

  /** test a fuzzy query */
  virtual void testFuzzy() ;

  /** test a simple phrase */
  virtual void testPhrase() ;

  /** test a simple phrase with various slop settings */
  virtual void testPhraseWithSlop() ;

  /** test a simple prefix */
  virtual void testPrefix() ;

  /** test some AND'd terms using '+' operator */
  virtual void testAND() ;

  /** test some AND'd phrases using '+' operator */
  virtual void testANDPhrase() ;

  /** test some AND'd terms (just using whitespace) */
  virtual void testANDImplicit() ;

  /** test some OR'd terms */
  virtual void testOR() ;

  /** test some OR'd terms (just using whitespace) */
  virtual void testORImplicit() ;

  /** test some OR'd phrases using '|' operator */
  virtual void testORPhrase() ;

  /** test negated term */
  virtual void testNOT() ;

  /** test crazy prefixes with multiple asterisks */
  virtual void testCrazyPrefixes1() ;

  /** test prefixes with some escaping */
  virtual void testCrazyPrefixes2() ;

  /** not a prefix query! the prefix operator is escaped */
  virtual void testTermInDisguise() ;

  // a number of test cases here have garbage/errors in
  // the syntax passed in to test that the query can
  // still be interpreted as a guess to what the human
  // input was trying to be

  virtual void testGarbageTerm() ;

  virtual void testGarbageEmpty() ;

  virtual void testGarbageAND() ;

  virtual void testGarbageOR() ;

  virtual void testGarbageNOT() ;

  virtual void testGarbagePhrase() ;

  virtual void testGarbageSubquery() ;

  virtual void testCompoundAnd() ;

  virtual void testCompoundOr() ;

  virtual void testComplex00() ;

  virtual void testComplex01() ;

  virtual void testComplex02() ;

  virtual void testComplex03() ;

  virtual void testComplex04() ;

  virtual void testComplex05() ;

  virtual void testComplex06() ;

  /** test a term with field weights */
  virtual void testWeightedTerm() ;

  /** test a more complex query with field weights */
  virtual void testWeightedOR() ;

  /** helper to parse a query with keyword analyzer across "field" */
private:
  std::shared_ptr<Query> parseKeyword(const std::wstring &text, int flags);

  /** test the ability to enable/disable phrase operator */
public:
  virtual void testDisablePhrase();

  /** test the ability to enable/disable prefix operator */
  virtual void testDisablePrefix();

  /** test the ability to enable/disable AND operator */
  virtual void testDisableAND();

  /** test the ability to enable/disable OR operator */
  virtual void testDisableOR();

  /** test the ability to enable/disable NOT operator */
  virtual void testDisableNOT();

  /** test the ability to enable/disable precedence operators */
  virtual void testDisablePrecedence();

  /** test the ability to enable/disable escape operators */
  virtual void testDisableEscape();

  virtual void testDisableWhitespace();

  virtual void testDisableFuzziness();

  virtual void testDisableSlop();

  // we aren't supposed to barf on any input...
  virtual void testRandomQueries() ;

  virtual void testRandomQueries2() ;

  virtual void testStarBecomesMatchAll() ;

protected:
  std::shared_ptr<TestSimpleQueryParser> shared_from_this()
  {
    return std::static_pointer_cast<TestSimpleQueryParser>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::simple
