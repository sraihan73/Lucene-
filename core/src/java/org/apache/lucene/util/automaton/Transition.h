#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::util::automaton
{

/** Holds one transition from an {@link Automaton}.  This is typically
 *  used temporarily when iterating through transitions by invoking
 *  {@link Automaton#initTransition} and {@link Automaton#getNextTransition}. */

class Transition : public std::enable_shared_from_this<Transition>
{
  GET_CLASS_NAME(Transition)

  /** Sole constructor. */
public:
  Transition();

  /** Source state. */
  int source = 0;

  /** Destination state. */
  int dest = 0;

  /** Minimum accepted label (inclusive). */
  int min = 0;

  /** Maximum accepted label (inclusive). */
  int max = 0;

  /** Remembers where we are in the iteration; init to -1 to provoke
   *  exception if nextTransition is called without first initTransition. */
  int transitionUpto = -1;

  virtual std::wstring toString();
};

} // namespace org::apache::lucene::util::automaton
