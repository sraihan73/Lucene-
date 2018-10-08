#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::analysis
{

/** Wraps a Reader, and can throw random or fixed
 *  exceptions, and spoon feed read chars. */

class MockReaderWrapper : public Reader
{
  GET_CLASS_NAME(MockReaderWrapper)

private:
  const std::shared_ptr<Reader> in_;
  const std::shared_ptr<Random> random;

  int excAtChar = -1;
  int readSoFar = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  bool throwExcNext_ = false;

public:
  MockReaderWrapper(std::shared_ptr<Random> random,
                    std::shared_ptr<Reader> in_);

  /** Throw an exception after reading this many chars. */
  virtual void throwExcAfterChar(int charUpto);

  virtual void throwExcNext();

  virtual ~MockReaderWrapper();

  int read(std::deque<wchar_t> &cbuf, int off,
           int len)  override;

  bool markSupported() override;

  bool ready() override;

  static bool isMyEvilException(std::runtime_error t);

protected:
  std::shared_ptr<MockReaderWrapper> shared_from_this()
  {
    return std::static_pointer_cast<MockReaderWrapper>(
        java.io.Reader::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
