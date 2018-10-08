#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class Automaton;
}

namespace org::apache::lucene::util::automaton
{
class RegExp;
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
namespace org::apache::lucene::util::automaton
{

/**
 * This exception is thrown when determinizing an automaton would result in one
 * has too many states.
 */
class TooComplexToDeterminizeException : public std::runtime_error
{
  GET_CLASS_NAME(TooComplexToDeterminizeException)
private:
  const std::shared_ptr<Automaton> automaton;
  const std::shared_ptr<RegExp> regExp;
  const int maxDeterminizedStates;

  /** Use this constructor when the RegExp failed to convert to an automaton. */
public:
  TooComplexToDeterminizeException(
      std::shared_ptr<RegExp> regExp,
      std::shared_ptr<TooComplexToDeterminizeException> cause);

  /** Use this constructor when the automaton failed to determinize. */
  TooComplexToDeterminizeException(std::shared_ptr<Automaton> automaton,
                                   int maxDeterminizedStates);

  /** Returns the automaton that caused this exception, if any. */
  virtual std::shared_ptr<Automaton> getAutomaton();

  /**
   * Return the RegExp that caused this exception if any.
   */
  virtual std::shared_ptr<RegExp> getRegExp();

  /** Get the maximum number of allowed determinized states. */
  virtual int getMaxDeterminizedStates();

protected:
  std::shared_ptr<TooComplexToDeterminizeException> shared_from_this()
  {
    return std::static_pointer_cast<TooComplexToDeterminizeException>(
        RuntimeException::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::automaton
