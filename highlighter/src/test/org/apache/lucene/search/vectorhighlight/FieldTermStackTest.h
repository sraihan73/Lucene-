#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

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

class FieldTermStackTest : public AbstractTestCase
{
  GET_CLASS_NAME(FieldTermStackTest)

public:
  virtual void test1Term() ;

  virtual void test2Terms() ;

  virtual void test1Phrase() ;

private:
  void makeIndex() ;

public:
  virtual void test1TermB() ;

  virtual void test2TermsB() ;

  virtual void test1PhraseB() ;

private:
  void makeIndexB() ;

public:
  virtual void test1PhraseShortMV() ;

  virtual void test1PhraseLongMV() ;

  virtual void test1PhraseMVB() ;

  virtual void testWildcard() ;

  virtual void testTermInfoComparisonConsistency();

private:
  template <typename T>
  void assertConsistentEquals(T a, T b);

  template <typename T>
  void assertConsistentLessThan(T a, T b);

protected:
  std::shared_ptr<FieldTermStackTest> shared_from_this()
  {
    return std::static_pointer_cast<FieldTermStackTest>(
        AbstractTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::vectorhighlight
