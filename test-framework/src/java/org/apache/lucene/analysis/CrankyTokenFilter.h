#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::analysis
{

/**
 * Throws IOException from random Tokenstream methods.
 * <p>
 * This can be used to simulate a buggy analyzer in IndexWriter,
 * where we must delete the document but not abort everything in the buffer.
 */
class CrankyTokenFilter final : public TokenFilter
{
  GET_CLASS_NAME(CrankyTokenFilter)
public:
  const std::shared_ptr<Random> random;
  int thingToDo = 0;

  /** Creates a new CrankyTokenFilter */
  CrankyTokenFilter(std::shared_ptr<TokenStream> input,
                    std::shared_ptr<Random> random);

  bool incrementToken()  override;

  void end()  override;

  void reset()  override;

  virtual ~CrankyTokenFilter();

protected:
  std::shared_ptr<CrankyTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<CrankyTokenFilter>(
        TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
