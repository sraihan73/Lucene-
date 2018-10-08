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

using org::antlr::v4::runtime::DefaultErrorStrategy;
using org::antlr::v4::runtime::Parser;
using org::antlr::v4::runtime::RecognitionException;
using org::antlr::v4::runtime::Token;

/**
 * Allows for proper error handling in the ANTLR 4 parser
 */
class JavascriptParserErrorStrategy : public DefaultErrorStrategy
{
  GET_CLASS_NAME(JavascriptParserErrorStrategy)
  /**
   * Ensures the ANTLR parser will throw an exception after the first error
   *
   * @param recognizer the parser being used
   * @param re the original exception from the parser
   */
public:
  void recover(std::shared_ptr<Parser> recognizer,
               std::shared_ptr<RecognitionException> re) override;

  /**
   * Ensures the ANTLR parser will throw an exception after the first error
   *
   * @param recognizer the parser being used
   * @return no actual return value
   * @throws RecognitionException not used as a ParseException wrapped in a
   * RuntimeException is thrown instead
   */
  std::shared_ptr<Token> recoverInline(
      std::shared_ptr<Parser> recognizer)  override;

  /**
   * Do not allow syncing after errors to ensure the ANTLR parser will throw an
   * exception
   *
   * @param recognizer the parser being used
   */
  void sync(std::shared_ptr<Parser> recognizer) override;

protected:
  std::shared_ptr<JavascriptParserErrorStrategy> shared_from_this()
  {
    return std::static_pointer_cast<JavascriptParserErrorStrategy>(
        org.antlr.v4.runtime.DefaultErrorStrategy::shared_from_this());
  }
};

} // namespace org::apache::lucene::expressions::js
