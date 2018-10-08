#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <typeinfo>
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
namespace org::apache::lucene::util
{

using namespace org::apache::lucene::analysis::tokenattributes;

class TestAttributeSource : public LuceneTestCase
{
  GET_CLASS_NAME(TestAttributeSource)

public:
  virtual void testCaptureState();

  virtual void testCloneAttributes();

  virtual void testDefaultAttributeFactory() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) public void
  // testInvalidArguments() throws Exception
  virtual void testInvalidArguments() ;

  virtual void testLUCENE_3042() ;

  virtual void testClonePayloadAttribute() ;

  virtual void testRemoveAllAttributes();

protected:
  std::shared_ptr<TestAttributeSource> shared_from_this()
  {
    return std::static_pointer_cast<TestAttributeSource>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
