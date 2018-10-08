#pragma once
#include "exceptionhelper.h"
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
namespace org::apache::lucene::index
{

/**
 * Encapsulates all required internal state to position the associated
 * {@link TermsEnum} without re-seeking.
 *
 * @see TermsEnum#seekExact(org.apache.lucene.util.BytesRef, TermState)
 * @see TermsEnum#termState()
 * @lucene.experimental
 */
class TermState : public std::enable_shared_from_this<TermState>,
                  public Cloneable
{
  GET_CLASS_NAME(TermState)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  TermState();

  /**
   * Copies the content of the given {@link TermState} to this instance
   *
   * @param other
   *          the TermState to copy
   */
public:
  virtual void copyFrom(std::shared_ptr<TermState> other) = 0;

  std::shared_ptr<TermState> clone() override;

  virtual std::wstring toString();
};

} // namespace org::apache::lucene::index
