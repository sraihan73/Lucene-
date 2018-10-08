#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::index
{
class RandomIndexWriter;
}
namespace org::apache::lucene::util::automaton
{
class CompiledAutomaton;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::index
{
class TermState;
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
namespace org::apache::lucene::index
{

using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({ "SimpleText", "Memory", "Direct" }) public
// class TestTermsEnum extends org.apache.lucene.util.LuceneTestCase
class TestTermsEnum : public LuceneTestCase
{

public:
  virtual void test() ;

private:
  void addDoc(std::shared_ptr<RandomIndexWriter> w,
              std::shared_ptr<std::deque<std::wstring>> terms,
              std::unordered_map<std::shared_ptr<BytesRef>, int> &termToID,
              int id) ;

  bool accepts(std::shared_ptr<CompiledAutomaton> c,
               std::shared_ptr<BytesRef> b);

  // Tests Terms.intersect
public:
  virtual void testIntersectRandom() ;

private:
  std::shared_ptr<Directory> d;
  std::shared_ptr<IndexReader> r;

  const std::wstring FIELD = L"field";

  std::shared_ptr<IndexReader>
  makeIndex(std::deque<std::wstring> &terms) ;

public:
  virtual ~TestTermsEnum();

private:
  int docFreq(std::shared_ptr<IndexReader> r,
              const std::wstring &term) ;

public:
  virtual void testEasy() ;

  // tests:
  //   - test same prefix has non-floor block and floor block (ie, has 2 long
  //   outputs on same term prefix)
  //   - term that's entirely in the index

  virtual void testFloorBlocks() ;

  virtual void testZeroTerms() ;

private:
  std::wstring getRandomString();

public:
  virtual void testRandomTerms() ;

  // sugar
private:
  bool seekExact(std::shared_ptr<TermsEnum> te,
                 const std::wstring &term) ;

  // sugar
  std::wstring next(std::shared_ptr<TermsEnum> te) ;

  std::shared_ptr<BytesRef>
  getNonExistTerm(std::deque<std::shared_ptr<BytesRef>> &terms);

private:
  class TermAndState : public std::enable_shared_from_this<TermAndState>
  {
    GET_CLASS_NAME(TermAndState)
  public:
    const std::shared_ptr<BytesRef> term;
    const std::shared_ptr<TermState> state;

    TermAndState(std::shared_ptr<BytesRef> term,
                 std::shared_ptr<TermState> state);
  };

private:
  void testRandomSeeks(
      std::shared_ptr<IndexReader> r,
      std::deque<std::wstring> &validTermStrings) ;

public:
  virtual void testIntersectBasic() ;
  virtual void testIntersectStartTerm() ;

  virtual void testIntersectEmptyString() ;

  // LUCENE-5667
  virtual void testCommonPrefixTerms() ;

  // Stresses out many-terms-in-root-block case:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testVaryingTermsPerSegment() throws
  // Exception
  virtual void testVaryingTermsPerSegment() ;

  // LUCENE-7576
  virtual void testIntersectRegexp() ;

  // LUCENE-7576
  virtual void testInvalidAutomatonTermsEnum() ;

protected:
  std::shared_ptr<TestTermsEnum> shared_from_this()
  {
    return std::static_pointer_cast<TestTermsEnum>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
