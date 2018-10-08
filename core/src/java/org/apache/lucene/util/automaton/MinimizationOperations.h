#pragma once
#include "stringhelper.h"
#include <deque>
#include <memory>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class Automaton;
}

namespace org::apache::lucene::util::automaton
{
class StateListNode;
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
 * Operations for minimizing automata.
 *
 * @lucene.experimental
 */
class MinimizationOperations final
    : public std::enable_shared_from_this<MinimizationOperations>
{
  GET_CLASS_NAME(MinimizationOperations)

private:
  MinimizationOperations();

  /**
   * Minimizes (and determinizes if not already deterministic) the given
   * automaton using Hopcroft's algorighm.
   * @param maxDeterminizedStates maximum number of states determinizing the
   *  automaton can result in.  Set higher to allow more complex queries and
   *  lower to prevent memory exhaustion.
   */
public:
  static std::shared_ptr<Automaton> minimize(std::shared_ptr<Automaton> a,
                                             int maxDeterminizedStates);

public:
  class IntPair final : public std::enable_shared_from_this<IntPair>
  {
    GET_CLASS_NAME(IntPair)

  public:
    const int n1, n2;

    IntPair(int n1, int n2);
  };

public:
  class StateList final : public std::enable_shared_from_this<StateList>
  {
    GET_CLASS_NAME(StateList)

  public:
    int size = 0;

    std::shared_ptr<StateListNode> first, last;

    std::shared_ptr<StateListNode> add(int q);
  };

public:
  class StateListNode final : public std::enable_shared_from_this<StateListNode>
  {
    GET_CLASS_NAME(StateListNode)

  public:
    const int q;

    std::shared_ptr<StateListNode> next, prev;

    const std::shared_ptr<StateList> sl;

    StateListNode(int q, std::shared_ptr<StateList> sl);

    void remove();
  };
};

} // namespace org::apache::lucene::util::automaton
