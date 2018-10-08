#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::util::automaton
{

// The following code was generated with the moman/finenight pkg
// This package is available under the MIT License, see NOTICE.txt
// for more details.

using ParametricDescription = org::apache::lucene::util::automaton::
    LevenshteinAutomata::ParametricDescription;

/** Parametric description for generating a Levenshtein automaton of degree 2,
    with transpositions as primitive edits */
class Lev2TParametricDescription : public ParametricDescription
{
  GET_CLASS_NAME(Lev2TParametricDescription)

public:
  int transition(int absState, int position, int deque) override;

  // 1 vectors; 3 states per deque; array length = 3
private:
  static std::deque<int64_t> const toStates0;
  static std::deque<int64_t> const offsetIncrs0;

  // 2 vectors; 5 states per deque; array length = 10
  static std::deque<int64_t> const toStates1;
  static std::deque<int64_t> const offsetIncrs1;

  // 4 vectors; 13 states per deque; array length = 52
  static std::deque<int64_t> const toStates2;
  static std::deque<int64_t> const offsetIncrs2;

  // 8 vectors; 28 states per deque; array length = 224
  static std::deque<int64_t> const toStates3;
  static std::deque<int64_t> const offsetIncrs3;

  // 16 vectors; 45 states per deque; array length = 720
  static std::deque<int64_t> const toStates4;
  static std::deque<int64_t> const offsetIncrs4;

  // 32 vectors; 45 states per deque; array length = 1440
  static std::deque<int64_t> const toStates5;
  static std::deque<int64_t> const offsetIncrs5;

  // state map_obj
  //   0 -> [(0, 0)]
  //   1 -> [(0, 2)]
  //   2 -> [(0, 1)]
  //   3 -> [(0, 1), (1, 1)]
  //   4 -> [(0, 2), (1, 2)]
  //   5 -> [t(0, 2), (0, 2), (1, 2), (2, 2)]
  //   6 -> [(0, 2), (2, 1)]
  //   7 -> [(0, 1), (2, 2)]
  //   8 -> [(0, 2), (2, 2)]
  //   9 -> [(0, 1), (1, 1), (2, 1)]
  //   10 -> [(0, 2), (1, 2), (2, 2)]
  //   11 -> [(0, 1), (2, 1)]
  //   12 -> [t(0, 1), (0, 1), (1, 1), (2, 1)]
  //   13 -> [(0, 2), (1, 2), (2, 2), (3, 2)]
  //   14 -> [t(0, 2), (0, 2), (1, 2), (2, 2), (3, 2)]
  //   15 -> [(0, 2), t(1, 2), (1, 2), (2, 2), (3, 2)]
  //   16 -> [(0, 2), (2, 1), (3, 1)]
  //   17 -> [(0, 1), t(1, 2), (2, 2), (3, 2)]
  //   18 -> [(0, 2), (3, 2)]
  //   19 -> [(0, 2), (1, 2), t(1, 2), (2, 2), (3, 2)]
  //   20 -> [t(0, 2), (0, 2), (1, 2), (3, 1)]
  //   21 -> [(0, 1), (1, 1), (3, 2)]
  //   22 -> [(0, 2), (2, 2), (3, 2)]
  //   23 -> [(0, 2), (1, 2), (3, 1)]
  //   24 -> [(0, 2), (1, 2), (3, 2)]
  //   25 -> [(0, 1), (2, 2), (3, 2)]
  //   26 -> [(0, 2), (3, 1)]
  //   27 -> [(0, 1), (3, 2)]
  //   28 -> [(0, 2), (2, 1), (4, 2)]
  //   29 -> [(0, 2), t(1, 2), (1, 2), (2, 2), (3, 2), (4, 2)]
  //   30 -> [(0, 2), (1, 2), (4, 2)]
  //   31 -> [(0, 2), (1, 2), (3, 2), (4, 2)]
  //   32 -> [(0, 2), (2, 2), (3, 2), (4, 2)]
  //   33 -> [(0, 2), (1, 2), t(2, 2), (2, 2), (3, 2), (4, 2)]
  //   34 -> [(0, 2), (1, 2), (2, 2), t(2, 2), (3, 2), (4, 2)]
  //   35 -> [(0, 2), (3, 2), (4, 2)]
  //   36 -> [(0, 2), t(2, 2), (2, 2), (3, 2), (4, 2)]
  //   37 -> [t(0, 2), (0, 2), (1, 2), (2, 2), (4, 2)]
  //   38 -> [(0, 2), (1, 2), (2, 2), (4, 2)]
  //   39 -> [t(0, 2), (0, 2), (1, 2), (2, 2), (3, 2), (4, 2)]
  //   40 -> [(0, 2), (1, 2), (2, 2), (3, 2), (4, 2)]
  //   41 -> [(0, 2), (4, 2)]
  //   42 -> [t(0, 2), (0, 2), (1, 2), (2, 2), t(2, 2), (3, 2), (4, 2)]
  //   43 -> [(0, 2), (2, 2), (4, 2)]
  //   44 -> [(0, 2), (1, 2), t(1, 2), (2, 2), (3, 2), (4, 2)]

public:
  Lev2TParametricDescription(int w);

protected:
  std::shared_ptr<Lev2TParametricDescription> shared_from_this()
  {
    return std::static_pointer_cast<Lev2TParametricDescription>(
        org.apache.lucene.util.automaton.LevenshteinAutomata
            .ParametricDescription::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::automaton
