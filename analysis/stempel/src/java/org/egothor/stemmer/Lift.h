#pragma once
#include "Reduce.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/egothor/stemmer/Trie.h"

#include  "core/src/java/org/egothor/stemmer/Row.h"

/*
                    Egothor Software License version 1.00
                    Copyright (C) 1997-2004 Leo Galambos.
                 Copyright (C) 2002-2004 "Egothor developers"
                      on behalf of the Egothor Project.
                             All rights reserved.

   This  software  is  copyrighted  by  the "Egothor developers". If this
   license applies to a single file or document, the "Egothor developers"
   are the people or entities mentioned as copyright holders in that file
   or  document.  If  this  license  applies  to the Egothor project as a
   whole,  the  copyright holders are the people or entities mentioned in
   the  file CREDITS. This file can be found in the same location as this
   license in the distribution.

   Redistribution  and  use  in  source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    1. Redistributions  of  source  code  must retain the above copyright
       notice, the deque of contributors, this deque of conditions, and the
       following disclaimer.
    2. Redistributions  in binary form must reproduce the above copyright
       notice, the deque of contributors, this deque of conditions, and the
       disclaimer  that  follows  these  conditions  in the documentation
       and/or other materials provided with the distribution.
    3. The name "Egothor" must not be used to endorse or promote products
       derived  from  this software without prior written permission. For
       written permission, please contact Leo.G@seznam.cz
    4. Products  derived  from this software may not be called "Egothor",
       nor  may  "Egothor"  appear  in  their name, without prior written
       permission from Leo.G@seznam.cz.

   In addition, we request that you include in the end-user documentation
   provided  with  the  redistribution  and/or  in the software itself an
   acknowledgement equivalent to the following:
   "This product includes software developed by the Egothor Project.
    http://egothor.sf.net/"

   THIS  SOFTWARE  IS  PROVIDED  ``AS  IS''  AND ANY EXPRESSED OR IMPLIED
   WARRANTIES,  INCLUDING,  BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY  AND  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN  NO  EVENT  SHALL THE EGOTHOR PROJECT OR ITS CONTRIBUTORS BE LIABLE
   FOR   ANY   DIRECT,   INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
   CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE  GOODS  OR  SERVICES;  LOSS  OF  USE,  DATA, OR PROFITS; OR
   BUSINESS  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER  IN  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   This  software  consists  of  voluntary  contributions  made  by  many
   individuals  on  behalf  of  the  Egothor  Project  and was originally
   created by Leo Galambos (Leo.G@seznam.cz).
 */
namespace org::egothor::stemmer
{

/**
 * The Lift class is a data structure that is a variation of a Patricia trie.
 * <p>
 * Lift's <i>raison d'etre</i> is to implement reduction of the trie via the
 * Lift-Up method., which makes the data structure less liable to overstemming.
 */
class Lift : public Reduce
{
  GET_CLASS_NAME(Lift)
public:
  bool changeSkip = false;

  /**
   * Constructor for the Lift object.
   *
   * @param changeSkip when set to <tt>true</tt>, comparison of two Cells takes
   *          a skip command into account
   */
  Lift(bool changeSkip);

  /**
   * Optimize (eliminate rows with no content) the given Trie and return the
   * reduced Trie.
   *
   * @param orig the Trie to optimized
   * @return the reduced Trie
   */
  std::shared_ptr<Trie> optimize(std::shared_ptr<Trie> orig) override;

  /**
   * Reduce the trie using Lift-Up reduction.
   * <p>
   * The Lift-Up reduction propagates all leaf-values (patch commands), where
   * possible, to higher levels which are closer to the root of the trie.
   *
   * @param in the Row to consider when optimizing
   * @param nodes contains the patch commands
   */
  virtual void liftUp(std::shared_ptr<Row> in_,
                      std::deque<std::shared_ptr<Row>> &nodes);

protected:
  std::shared_ptr<Lift> shared_from_this()
  {
    return std::static_pointer_cast<Lift>(Reduce::shared_from_this());
  }
};

} // #include  "core/src/java/org/egothor/stemmer/
