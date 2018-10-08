#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldFragList.h"

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

using Query = org::apache::lucene::search::Query;

class SimpleFragmentsBuilderTest : public AbstractTestCase
{
  GET_CLASS_NAME(SimpleFragmentsBuilderTest)

public:
  virtual void test1TermIndex() ;

  virtual void test2Frags() ;

  virtual void test3Frags() ;

  virtual void testTagsAndEncoder() ;

private:
  std::shared_ptr<FieldFragList>
  ffl(std::shared_ptr<Query> query,
      const std::wstring &indexValue) ;

public:
  virtual void test1PhraseShortMV() ;

  virtual void test1PhraseLongMV() ;

  virtual void test1PhraseLongMVB() ;

  virtual void testUnstoredField() ;

protected:
  virtual void makeUnstoredIndex() ;

public:
  virtual void test1StrMV() ;

  virtual void testMVSeparator() ;

  virtual void testDiscreteMultiValueHighlighting() ;

  virtual void
  testRandomDiscreteMultiValueHighlighting() ;

private:
  std::wstring
  getRandomValue(std::deque<std::wstring> &randomValues,
                 std::unordered_map<std::wstring, Set<int>> &valueToDocId,
                 int docId);

private:
  class Doc : public std::enable_shared_from_this<Doc>
  {
    GET_CLASS_NAME(Doc)

  public:
    std::deque<std::deque<std::wstring>> const fieldValues;

  private:
    Doc(std::deque<std::deque<std::wstring>> &fieldValues);
  };

protected:
  std::shared_ptr<SimpleFragmentsBuilderTest> shared_from_this()
  {
    return std::static_pointer_cast<SimpleFragmentsBuilderTest>(
        AbstractTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
