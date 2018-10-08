#pragma once
#include "stringhelper.h"
#include <deque>
#include <memory>
#include <stdexcept>
#include <type_traits>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::vectorhighlight
{
class FieldPhraseList;
}

namespace org::apache::lucene::search::vectorhighlight
{
class WeightedPhraseInfo;
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
namespace org::apache::lucene::search::vectorhighlight
{

using WeightedPhraseInfo = org::apache::lucene::search::vectorhighlight::
    FieldPhraseList::WeightedPhraseInfo;

class FieldPhraseListTest : public AbstractTestCase
{
  GET_CLASS_NAME(FieldPhraseListTest)

public:
  virtual void test1TermIndex() ;

  virtual void test2TermsIndex() ;

  virtual void test1PhraseIndex() ;

  virtual void test1PhraseIndexB() ;

  virtual void test2ConcatTermsIndexB() ;

  virtual void test2Terms1PhraseIndex() ;

  virtual void testPhraseSlop() ;

  virtual void test2PhrasesOverlap() ;

  virtual void test3TermsPhrase() ;

  virtual void testSearchLongestPhrase() ;

  virtual void test1PhraseShortMV() ;

  virtual void test1PhraseLongMV() ;

  virtual void test1PhraseLongMVB() ;

  /* This test shows a big speedup from limiting the number of analyzed phrases
   * in this bad case for FieldPhraseList */
  /* But it is not reliable as a unit test since it is timing-dependent
  public void testManyRepeatedTerms() throws Exception {
      long t = System.currentTimeMillis();
      testManyTermsWithLimit (-1);
      long t1 = System.currentTimeMillis();
      testManyTermsWithLimit (1);
      long t2 = System.currentTimeMillis();
      assertTrue (t2-t1 * 1000 < t1-t);
  }
  private void testManyTermsWithLimit (int limit) throws Exception {
      StringBuilder buf = new StringBuilder ();
      for (int i = 0; i < 16000; i++) {
          buf.append("a b c ");
      }
      make1d1fIndex( buf.toString());

      Query query = tq("a");
      FieldQuery fq = new FieldQuery( query, true, true );
      FieldTermStack stack = new FieldTermStack( reader, 0, F, fq );
      FieldPhraseList fpl = new FieldPhraseList( stack, fq, limit);
      if (limit < 0 || limit > 16000)
          assertEquals( 16000, fpl.phraseList.size() );
      else
          assertEquals( limit, fpl.phraseList.size() );
      assertEquals( "a(1.0)((0,1))", fpl.phraseList.get( 0 ).toString() );
  }
  */

  virtual void testWeightedPhraseInfoComparisonConsistency();

  virtual void testToffsComparisonConsistency();

private:
  std::shared_ptr<WeightedPhraseInfo> newInfo(int startOffset, int endOffset,
                                              float boost);

  template <typename T>
  void assertConsistentEquals(T a, T b);

  template <typename T>
  void assertConsistentLessThan(T a, T b);

protected:
  std::shared_ptr<FieldPhraseListTest> shared_from_this()
  {
    return std::static_pointer_cast<FieldPhraseListTest>(
        AbstractTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::vectorhighlight
