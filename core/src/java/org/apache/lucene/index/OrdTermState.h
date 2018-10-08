#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class TermState;
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
namespace org::apache::lucene::index
{

/**
 * An ordinal based {@link TermState}
 *
 * @lucene.experimental
 */
class OrdTermState : public TermState
{
  GET_CLASS_NAME(OrdTermState)
  /** Term ordinal, i.e. its position in the full deque of
   *  sorted terms. */
public:
  int64_t ord = 0;

  /** Sole constructor. */
  OrdTermState();

  void copyFrom(std::shared_ptr<TermState> other) override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<OrdTermState> shared_from_this()
  {
    return std::static_pointer_cast<OrdTermState>(
        TermState::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
