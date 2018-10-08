#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/automaton/CompiledAutomaton.h"

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
namespace org::apache::lucene::util::automaton
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestCompiledAutomaton : public LuceneTestCase
{
  GET_CLASS_NAME(TestCompiledAutomaton)

private:
  std::shared_ptr<CompiledAutomaton> build(int maxDeterminizedStates,
                                           std::deque<std::wstring> &strings);

  void testFloor(std::shared_ptr<CompiledAutomaton> c,
                 const std::wstring &input, const std::wstring &expected);

  void testTerms(int maxDeterminizedStates,
                 std::deque<std::wstring> &terms) ;

public:
  virtual void testRandom() ;

private:
  std::wstring randomString();

public:
  virtual void testBasic() ;

  // LUCENE-6367
  virtual void testBinaryAll() ;

  // LUCENE-6367
  virtual void testUnicodeAll() ;

  // LUCENE-6367
  virtual void testBinarySingleton() ;

  // LUCENE-6367
  virtual void testUnicodeSingleton() ;

protected:
  std::shared_ptr<TestCompiledAutomaton> shared_from_this()
  {
    return std::static_pointer_cast<TestCompiledAutomaton>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/automaton/
