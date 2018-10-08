#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::search
{
class DoubleValuesSource;
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
namespace org::apache::lucene::expressions
{

using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.expressions.js.VariableContext.Type.INT_INDEX;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.expressions.js.VariableContext.Type.MEMBER;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.expressions.js.VariableContext.Type.STR_INDEX;

/** simple demo of using expressions */
class TestDemoExpressions : public LuceneTestCase
{
  GET_CLASS_NAME(TestDemoExpressions)
public:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<DirectoryReader> reader;
  std::shared_ptr<Directory> dir;

  void setUp()  override;

  void tearDown()  override;

  /** an example of how to rank by an expression */
  virtual void test() ;

  /** tests the returned sort values are correct */
  virtual void testSortValues() ;

  /** tests same binding used more than once in an expression */
  virtual void testTwoOfSameBinding() ;

  /** Uses variables with $ */
  virtual void testDollarVariable() ;

  /** tests expression referring to another expression */
  virtual void testExpressionRefersToExpression() ;

  /** tests huge amounts of variables in the expression */
  virtual void testLotsOfBindings() ;

private:
  void doTestLotsOfBindings(int n) ;

public:
  virtual void testDistanceSort() ;

  virtual void testStaticExtendedVariableExample() ;

  virtual void testDynamicExtendedVariableExample() ;

private:
  class BindingsAnonymousInnerClass : public Bindings
  {
    GET_CLASS_NAME(BindingsAnonymousInnerClass)
  private:
    std::shared_ptr<TestDemoExpressions> outerInstance;

  public:
    BindingsAnonymousInnerClass(
        std::shared_ptr<TestDemoExpressions> outerInstance);

    std::shared_ptr<DoubleValuesSource>
    getDoubleValuesSource(const std::wstring &name) override;

  protected:
    std::shared_ptr<BindingsAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<BindingsAnonymousInnerClass>(
          Bindings::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestDemoExpressions> shared_from_this()
  {
    return std::static_pointer_cast<TestDemoExpressions>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::expressions
