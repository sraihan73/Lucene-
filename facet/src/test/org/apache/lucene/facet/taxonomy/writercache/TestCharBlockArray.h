#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet::taxonomy::writercache
{
class CharBlockArray;
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
namespace org::apache::lucene::facet::taxonomy::writercache
{

using FacetTestCase = org::apache::lucene::facet::FacetTestCase;

class TestCharBlockArray : public FacetTestCase
{
  GET_CLASS_NAME(TestCharBlockArray)

public:
  virtual void testArray() ;

private:
  static void assertEqualsInternal(const std::wstring &msg,
                                   std::shared_ptr<StringBuilder> expected,
                                   std::shared_ptr<CharBlockArray> actual);

protected:
  std::shared_ptr<TestCharBlockArray> shared_from_this()
  {
    return std::static_pointer_cast<TestCharBlockArray>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::taxonomy::writercache
