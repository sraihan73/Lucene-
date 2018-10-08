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
namespace org::apache::lucene::facet::taxonomy
{

using FacetTestCase = org::apache::lucene::facet::FacetTestCase;

class TestLRUHashMap : public FacetTestCase
{
  GET_CLASS_NAME(TestLRUHashMap)
  // testLRU() tests that the specified size limit is indeed honored, and
  // the remaining objects in the map_obj are indeed those that have been most
  // recently used
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLRU() throws Exception
  virtual void testLRU() ;

protected:
  std::shared_ptr<TestLRUHashMap> shared_from_this()
  {
    return std::static_pointer_cast<TestLRUHashMap>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
