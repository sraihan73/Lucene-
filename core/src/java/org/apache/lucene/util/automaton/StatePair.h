#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

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
 * Pair of states.
 *
 * @lucene.experimental
 */
class StatePair : public std::enable_shared_from_this<StatePair>
{
  GET_CLASS_NAME(StatePair)
public:
  int s = 0;
  int s1 = 0;
  int s2 = 0;

  StatePair(int s, int s1, int s2);

  /**
   * Constructs a new state pair.
   *
   * @param s1 first state
   * @param s2 second state
   */
  StatePair(int s1, int s2);

  /**
   * Checks for equality.
   *
   * @param obj object to compare with
   * @return true if <tt>obj</tt> represents the same pair of states as this
   *         pair
   */
  bool equals(std::any obj) override;

  /**
   * Returns hash code.
   *
   * @return hash code
   */
  virtual int hashCode();

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/util/automaton/
