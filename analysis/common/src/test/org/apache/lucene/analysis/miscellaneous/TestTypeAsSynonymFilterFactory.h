#pragma once
#include "../util/BaseTokenStreamFactoryTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Token;
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

namespace org::apache::lucene::analysis::miscellaneous
{

using Token = org::apache::lucene::analysis::Token;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

class TestTypeAsSynonymFilterFactory : public BaseTokenStreamFactoryTestCase
{
  GET_CLASS_NAME(TestTypeAsSynonymFilterFactory)

private:
  static std::deque<std::shared_ptr<Token>> const TOKENS;

public:
  virtual void testBasic() ;

  virtual void testPrefix() ;

private:
  static std::shared_ptr<Token> token(const std::wstring &term,
                                      const std::wstring &type);

protected:
  std::shared_ptr<TestTypeAsSynonymFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestTypeAsSynonymFilterFactory>(
        org.apache.lucene.analysis.util
            .BaseTokenStreamFactoryTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous