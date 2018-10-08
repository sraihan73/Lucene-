#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
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
using org::objectweb::asm_::Opcodes;

/** Tests customing the function map_obj */
class TestCustomFunctions : public LuceneTestCase
{
  GET_CLASS_NAME(TestCustomFunctions)
private:
  static double DELTA;

  /** empty deque of methods */
public:
  virtual void testEmpty() ;

  /** using the default map_obj explicitly */
  virtual void testDefaultList() ;

  static double zeroArgMethod();

  /** tests a method with no arguments */
  virtual void testNoArgMethod() ;

  static double oneArgMethod(double arg1);

  /** tests a method with one arguments */
  virtual void testOneArgMethod() ;

  static double threeArgMethod(double arg1, double arg2, double arg3);

  /** tests a method with three arguments */
  virtual void testThreeArgMethod() ;

  /** tests a map_obj with 2 functions */
  virtual void testTwoMethods() ;

  /** tests invalid methods that are not allowed to become variables to be
   * mapped */
  virtual void testInvalidVariableMethods();

  static std::wstring bogusReturnType();

  /** wrong return type: must be double */
  virtual void testWrongReturnType() ;

  static double bogusParameterType(const std::wstring &s);

  /** wrong param type: must be doubles */
  virtual void testWrongParameterType() ;

  virtual double nonStaticMethod();

  /** wrong modifiers: must be static */
  virtual void testWrongNotStatic() ;

  static double nonPublicMethod();

  /** wrong modifiers: must be public */
  virtual void testWrongNotPublic() ;

public:
  class NestedNotPublic : public std::enable_shared_from_this<NestedNotPublic>
  {
    GET_CLASS_NAME(NestedNotPublic)
  public:
    static double method();
  };

  /** wrong class modifiers: class containing method is not public */
public:
  virtual void testWrongNestedNotPublic() ;

  /** Classloader that can be used to create a fake static class that has one
   * method returning a static var */
public:
  class Loader final : public ClassLoader, public Opcodes
  {
    GET_CLASS_NAME(Loader)
  public:
    Loader(std::shared_ptr<ClassLoader> parent);

    std::type_info createFakeClass();

  protected:
    std::shared_ptr<Loader> shared_from_this()
    {
      return std::static_pointer_cast<Loader>(ClassLoader::shared_from_this());
    }
  };

  /** uses this test with a different classloader and tries to
   * register it using the default classloader, which should fail */
public:
  virtual void testClassLoader() ;

  static std::wstring MESSAGE;

public:
  class StaticThrowingException
      : public std::enable_shared_from_this<StaticThrowingException>
  {
    GET_CLASS_NAME(StaticThrowingException)
  public:
    static double method();
  };

  /** the method throws an exception. We should check the stack trace that it
   * contains the source code of the expression as file name. */
public:
  virtual void testThrowingException() ;

  /** test that namespaces work with custom expressions:: */
  virtual void testNamespaces() ;

protected:
  std::shared_ptr<TestCustomFunctions> shared_from_this()
  {
    return std::static_pointer_cast<TestCustomFunctions>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/expressions/js/
