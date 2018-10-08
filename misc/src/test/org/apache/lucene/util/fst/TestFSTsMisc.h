#pragma once
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/MockDirectoryWrapper.h"

#include  "core/src/java/org/apache/lucene/util/IntsRef.h"

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
namespace org::apache::lucene::util::fst
{

using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using IntsRef = org::apache::lucene::util::IntsRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.fst.FSTTester.getRandomString;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.fst.FSTTester.toIntsRef;

class TestFSTsMisc : public LuceneTestCase
{
  GET_CLASS_NAME(TestFSTsMisc)

private:
  std::shared_ptr<MockDirectoryWrapper> dir;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testRandomWords() ;

private:
  void testRandomWords(int maxNumWords, int numIter) ;

  void doTest(int inputMode,
              std::deque<std::shared_ptr<IntsRef>> &terms) ;

private:
  class FSTTesterAnonymousInnerClass : public FSTTester<std::any>
  {
    GET_CLASS_NAME(FSTTesterAnonymousInnerClass)
  private:
    std::shared_ptr<TestFSTsMisc> outerInstance;

  public:
    FSTTesterAnonymousInnerClass(std::shared_ptr<TestFSTsMisc> outerInstance,
                                 std::shared_ptr<Random> random,
                                 std::shared_ptr<MockDirectoryWrapper> dir,
                                 int inputMode);

  protected:
    bool outputsEqual(std::any output1, std::any output2) override;

  protected:
    std::shared_ptr<FSTTesterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FSTTesterAnonymousInnerClass>(
          FSTTester<Object>::shared_from_this());
    }
  };

public:
  virtual void testListOfOutputs() ;

  virtual void testListOfOutputsEmptyString() ;

protected:
  std::shared_ptr<TestFSTsMisc> shared_from_this()
  {
    return std::static_pointer_cast<TestFSTsMisc>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/fst/
