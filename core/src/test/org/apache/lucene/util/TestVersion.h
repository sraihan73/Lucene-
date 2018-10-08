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
namespace org::apache::lucene::util
{

class TestVersion : public LuceneTestCase
{
  GET_CLASS_NAME(TestVersion)

public:
  virtual void testOnOrAfter() ;

  virtual void testToString();

  virtual void testParseLeniently() ;

  virtual void testParseLenientlyExceptions();

  virtual void testParseLenientlyOnAllConstants() ;

  virtual void testParse() ;

  virtual void testForwardsCompatibility() ;

  virtual void testParseExceptions();

  virtual void testDeprecations() ;

  virtual void testLatestVersionCommonBuild();

  virtual void testEqualsHashCode() ;

protected:
  std::shared_ptr<TestVersion> shared_from_this()
  {
    return std::static_pointer_cast<TestVersion>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
