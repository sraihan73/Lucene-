#pragma once
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
namespace org::apache::lucene::facet::taxonomy
{

using FacetTestCase = org::apache::lucene::facet::FacetTestCase;

class TestFacetLabel : public FacetTestCase
{
  GET_CLASS_NAME(TestFacetLabel)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBasic()
  virtual void testBasic();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testToString()
  virtual void testToString();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetComponent()
  virtual void testGetComponent();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testDefaultConstructor()
  virtual void testDefaultConstructor();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSubPath()
  virtual void testSubPath();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEquals()
  virtual void testEquals();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testHashCode()
  virtual void testHashCode();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLongHashCode()
  virtual void testLongHashCode();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testArrayConstructor()
  virtual void testArrayConstructor();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCompareTo()
  virtual void testCompareTo();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEmptyNullComponents() throws Exception
  virtual void testEmptyNullComponents() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLongPath() throws Exception
  virtual void testLongPath() ;

protected:
  std::shared_ptr<TestFacetLabel> shared_from_this()
  {
    return std::static_pointer_cast<TestFacetLabel>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::taxonomy
