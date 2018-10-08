#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"

namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
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
namespace org::apache::lucene::search::suggest::analyzing
{

using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using FST = org::apache::lucene::util::fst::FST;

// TODO: move to core?  nobody else uses it yet though...

/**
 * Exposes a utility method to enumerate all paths
 * intersecting an {@link Automaton} with an {@link FST}.
 */
class FSTUtil : public std::enable_shared_from_this<FSTUtil>
{
  GET_CLASS_NAME(FSTUtil)

private:
  FSTUtil();

  /** Holds a pair (automaton, fst) of states and accumulated output in the
   * intersected machine. */
public:
  template <typename T>
  class Path final : public std::enable_shared_from_this<Path>
  {
    GET_CLASS_NAME(Path)

    /** Node in the automaton where path ends: */
  public:
    const int state;

    /** Node in the FST where path ends: */
    const std::shared_ptr<FST::Arc<T>> fstNode;

    /** Output of the path so far: */
    const T output;

    /** Input of the path so far: */
    const std::shared_ptr<IntsRefBuilder> input;

    /** Sole constructor. */
    Path(int state, std::shared_ptr<FST::Arc<T>> fstNode, T output,
         std::shared_ptr<IntsRefBuilder> input)
        : state(state), fstNode(fstNode), output(output), input(input)
    {
    }
  };

  /**
   * Enumerates all minimal prefix paths in the automaton that also intersect
   * the FST, accumulating the FST end node and output for each path.
   */
public:
  template <typename T>
  static std::deque<Path<T>>
  intersectPrefixPaths(std::shared_ptr<Automaton> a,
                       std::shared_ptr<FST<T>> fst) ;
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/analyzing/
