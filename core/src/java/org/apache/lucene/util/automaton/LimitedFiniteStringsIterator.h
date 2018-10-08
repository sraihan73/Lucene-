#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class Automaton;
}

namespace org::apache::lucene::util
{
class IntsRef;
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

using IntsRef = org::apache::lucene::util::IntsRef;

/**
 * {@link FiniteStringsIterator} which limits the number of iterated accepted
 * strings. If more than <code>limit</code> strings are accepted, the first
 * <code>limit</code> strings found are returned.
 *
 * <p>If the {@link Automaton} has cycles then this iterator may throw an {@code
 * IllegalArgumentException}, but this is not guaranteed!
 *
 * <p>Be aware that the iteration order is implementation dependent
 * and may change across releases.
 *
 * @lucene.experimental
 */
class LimitedFiniteStringsIterator : public FiniteStringsIterator
{
  GET_CLASS_NAME(LimitedFiniteStringsIterator)
  /**
   * Maximum number of finite strings to create.
   */
private:
  int limit = std::numeric_limits<int>::max();

  /**
   * Number of generated finite strings.
   */
  int count = 0;

  /**
   * Constructor.
   *
   * @param a Automaton to create finite string from.
   * @param limit Maximum number of finite strings to create, or -1 for
   * infinite.
   */
public:
  LimitedFiniteStringsIterator(std::shared_ptr<Automaton> a, int limit);

  std::shared_ptr<IntsRef> next() override;

  /**
   * Number of iterated finite strings.
   */
  virtual int size();

protected:
  std::shared_ptr<LimitedFiniteStringsIterator> shared_from_this()
  {
    return std::static_pointer_cast<LimitedFiniteStringsIterator>(
        FiniteStringsIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::automaton
