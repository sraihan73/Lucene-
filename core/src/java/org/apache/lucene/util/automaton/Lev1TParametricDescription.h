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

/** Parametric description for generating a Levenshtein automaton of degree 1,
    with transpositions as primitive edits */
class Lev1TParametricDescription : public ParametricDescription
{
  GET_CLASS_NAME(Lev1TParametricDescription)

public:
  int transition(int absState, int position, int deque) override;

  // 1 vectors; 2 states per deque; array length = 2
private:
  static std::deque<int64_t> const toStates0;
  static std::deque<int64_t> const offsetIncrs0;

  // 2 vectors; 3 states per deque; array length = 6
  static std::deque<int64_t> const toStates1;
  static std::deque<int64_t> const offsetIncrs1;

  // 4 vectors; 6 states per deque; array length = 24
  static std::deque<int64_t> const toStates2;
  static std::deque<int64_t> const offsetIncrs2;

  // 8 vectors; 6 states per deque; array length = 48
  static std::deque<int64_t> const toStates3;
  static std::deque<int64_t> const offsetIncrs3;

  // state map_obj
  //   0 -> [(0, 0)]
  //   1 -> [(0, 1)]
  //   2 -> [(0, 1), (1, 1)]
  //   3 -> [(0, 1), (2, 1)]
  //   4 -> [t(0, 1), (0, 1), (1, 1), (2, 1)]
  //   5 -> [(0, 1), (1, 1), (2, 1)]

public:
  Lev1TParametricDescription(int w);

protected:
  std::shared_ptr<Lev1TParametricDescription> shared_from_this()
  {
    return std::static_pointer_cast<Lev1TParametricDescription>(
        org.apache.lucene.util.automaton.LevenshteinAutomata
            .ParametricDescription::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::automaton
