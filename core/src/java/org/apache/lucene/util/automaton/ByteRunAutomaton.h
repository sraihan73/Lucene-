#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class Automaton;
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
 * Automaton representation for matching UTF-8 byte[].
 */
class ByteRunAutomaton : public RunAutomaton
{
  GET_CLASS_NAME(ByteRunAutomaton)

  /** Converts incoming automaton to byte-based (UTF32ToUTF8) first */
public:
  ByteRunAutomaton(std::shared_ptr<Automaton> a);

  /** expert: if isBinary is true, the input is already byte-based */
  ByteRunAutomaton(std::shared_ptr<Automaton> a, bool isBinary,
                   int maxDeterminizedStates);

  /**
   * Returns true if the given byte array is accepted by this automaton
   */
  virtual bool run(std::deque<char> &s, int offset, int length);

protected:
  std::shared_ptr<ByteRunAutomaton> shared_from_this()
  {
    return std::static_pointer_cast<ByteRunAutomaton>(
        RunAutomaton::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::automaton
