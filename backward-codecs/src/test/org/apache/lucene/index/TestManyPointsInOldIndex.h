#pragma once
#include "../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// LUCENE-7501
class TestManyPointsInOldIndex : public LuceneTestCase
{
  GET_CLASS_NAME(TestManyPointsInOldIndex)

  // To regenerate the back index zip:
  //
  // Compile:
  //   1) temporarily remove 'extends LuceneTestCase' above (else java doesn't
  //   see our static void main) 2) ant compile-test
  //
  // Run:
  //   1) java -cp
  //   ../build/backward-codecs/classes/test:../build/core/classes/java
  //   org.apache.lucene.index.TestManyPointsInOldIndex
  //
  //  cd manypointsindex
  //  zip manypointsindex.zip *

  static void main(std::deque<std::wstring> &args) ;

public:
  virtual void testCheckOldIndex() ;

protected:
  std::shared_ptr<TestManyPointsInOldIndex> shared_from_this()
  {
    return std::static_pointer_cast<TestManyPointsInOldIndex>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
