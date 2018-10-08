#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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

class TestRegExp : public LuceneTestCase
{
  GET_CLASS_NAME(TestRegExp)

  /**
   * Simple smoke test for regular expression.
   */
public:
  virtual void testSmoke();

  /**
   * Compiles a regular expression that is prohibitively expensive to
   * determinize and expexts to catch an exception for it.
   */
  virtual void testDeterminizeTooManyStates();

  virtual void testSerializeTooManyStatesToRepeat() ;

  // LUCENE-6713
  virtual void
  testSerializeTooManyStatesToDeterminizeExc() ;

  // LUCENE-6046
  virtual void testRepeatWithEmptyString() ;

  virtual void testRepeatWithEmptyLanguage() ;

protected:
  std::shared_ptr<TestRegExp> shared_from_this()
  {
    return std::static_pointer_cast<TestRegExp>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/automaton/
