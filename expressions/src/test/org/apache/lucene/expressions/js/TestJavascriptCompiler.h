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
namespace org::apache::lucene::expressions::js
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestJavascriptCompiler : public LuceneTestCase
{
  GET_CLASS_NAME(TestJavascriptCompiler)

public:
  virtual void testValidCompiles() ;

  virtual void testValidVariables() ;

  virtual void
  doTestValidVariable(const std::wstring &variable) ;

  virtual void
  doTestValidVariable(const std::wstring &variable,
                      const std::wstring &output) ;

  virtual void testInvalidVariables() ;

  virtual void doTestInvalidVariable(const std::wstring &variable);

  virtual void testInvalidLexer() ;

  virtual void testInvalidCompiles() ;

  virtual void testEmpty();

  virtual void testNull() ;

  virtual void testWrongArity() ;

  virtual void testVariableNormalization() ;

protected:
  std::shared_ptr<TestJavascriptCompiler> shared_from_this()
  {
    return std::static_pointer_cast<TestJavascriptCompiler>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::expressions::js
