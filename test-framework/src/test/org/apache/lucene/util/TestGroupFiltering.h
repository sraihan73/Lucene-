#pragma once
#include "stringhelper.h"
#include <memory>

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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestGroupFiltering : public LuceneTestCase
{
  GET_CLASS_NAME(TestGroupFiltering)
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @TestGroup(enabled = false) public @interface Foo
  //  {
  //  }
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @TestGroup(enabled = false) public @interface Bar
  //  {
  //  }
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @TestGroup(enabled = false) public @interface Jira
  //  {
  //    std::wstring bug();
  //  }

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Foo public void testFoo()
  virtual void testFoo();
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Foo @Bar public void testFooBar()
  virtual void testFooBar();
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Bar public void testBar()
  virtual void testBar();
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Jira(bug = "JIRA bug reference") public void testJira()
  virtual void testJira();

protected:
  std::shared_ptr<TestGroupFiltering> shared_from_this()
  {
    return std::static_pointer_cast<TestGroupFiltering>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
