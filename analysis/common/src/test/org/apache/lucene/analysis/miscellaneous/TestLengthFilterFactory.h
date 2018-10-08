#pragma once
#include "../util/BaseTokenStreamFactoryTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::analysis::miscellaneous
{

using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

class TestLengthFilterFactory : public BaseTokenStreamFactoryTestCase
{
  GET_CLASS_NAME(TestLengthFilterFactory)

public:
  virtual void testPositionIncrements() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

  /** Test that invalid arguments result in exception */
  virtual void testInvalidArguments() ;

protected:
  std::shared_ptr<TestLengthFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestLengthFilterFactory>(
        org.apache.lucene.analysis.util
            .BaseTokenStreamFactoryTestCase::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
