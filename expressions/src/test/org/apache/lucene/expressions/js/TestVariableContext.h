#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::expressions::js
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.expressions.js.VariableContext.Type.MEMBER;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.expressions.js.VariableContext.Type.STR_INDEX;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.expressions.js.VariableContext.Type.INT_INDEX;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.expressions.js.VariableContext.Type.METHOD;

class TestVariableContext : public LuceneTestCase
{
  GET_CLASS_NAME(TestVariableContext)

public:
  virtual void testSimpleVar();

  virtual void testEmptyString();

  virtual void testUnescapeString();

  virtual void testMember();

  virtual void testMemberFollowedByMember();

  virtual void testMemberFollowedByIntArray();

  virtual void testMethodWithMember();

  virtual void testMethodWithStrIndex();

  virtual void testMethodWithNumericalIndex();

protected:
  std::shared_ptr<TestVariableContext> shared_from_this()
  {
    return std::static_pointer_cast<TestVariableContext>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/expressions/js/
