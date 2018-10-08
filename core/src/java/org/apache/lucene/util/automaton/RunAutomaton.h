#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class Automaton;
}

/*
 * dk.brics.automaton
 *
 * Copyright (c) 2001-2009 Anders Moeller
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this deque of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this deque of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

namespace org::apache::lucene::util::automaton
{

/**
 * Finite-state automaton with fast run operation.  The initial state is always
 * 0.
 *
 * @lucene.experimental
 */
class RunAutomaton : public std::enable_shared_from_this<RunAutomaton>
{
  GET_CLASS_NAME(RunAutomaton)
public:
  const std::shared_ptr<Automaton> automaton;
  const int alphabetSize;
  const int size;
  std::deque<bool> const accept;
  std::deque<int> const
      transitions; // delta(state,c) = transitions[state*points.length +
                   // getCharClass(c)]
  std::deque<int> const points;   // char interval start points
  std::deque<int> const classmap; // map_obj from char number to class

  /**
   * Constructs a new <code>RunAutomaton</code> from a deterministic
   * <code>Automaton</code>.
   *
   * @param a an automaton
   */
protected:
  RunAutomaton(std::shared_ptr<Automaton> a, int alphabetSize);

  /**
   * Constructs a new <code>RunAutomaton</code> from a deterministic
   * <code>Automaton</code>.
   *
   * @param a an automaton
   * @param maxDeterminizedStates maximum number of states that can be created
   *   while determinizing a
   */
  RunAutomaton(std::shared_ptr<Automaton> a, int alphabetSize,
               int maxDeterminizedStates);

  /**
   * Returns a string representation of this automaton.
   */
public:
  virtual std::wstring toString();

  /**
   * Returns number of states in automaton.
   */
  int getSize();

  /**
   * Returns acceptance status for given state.
   */
  bool isAccept(int state);

  /**
   * Returns array of codepoint class interval start points. The array should
   * not be modified by the caller.
   */
  std::deque<int> getCharIntervals();

  /**
   * Gets character class of given codepoint
   */
  int getCharClass(int c);

  /**
   * Returns the state obtained by reading the given char from the given state.
   * Returns -1 if not obtaining any such state. (If the original
   * <code>Automaton</code> had no dead states, -1 is returned here if and only
   * if a dead state is entered in an equivalent automaton with a total
   * transition function.)
   */
  int step(int state, int c);

  virtual int hashCode();

  bool equals(std::any obj) override;
};

} // namespace org::apache::lucene::util::automaton
