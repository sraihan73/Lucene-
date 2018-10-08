#pragma once
#include "MultiTrie.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::egothor::stemmer
{
class Reduce;
}

namespace org::egothor::stemmer
{
class Trie;
}

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
 * The MultiTrie is a Trie of Tries.
 * <p>
 * It stores words and their associated patch commands. The MultiTrie handles
 * patch commands broken into their constituent parts, as a MultiTrie does, but
 * the commands are delimited by the skip command.
 */
class MultiTrie2 : public MultiTrie
{
  GET_CLASS_NAME(MultiTrie2)
  /**
   * Constructor for the MultiTrie object.
   *
   * @param is the input stream
   * @exception IOException if an I/O error occurs
   */
public:
  MultiTrie2(std::shared_ptr<DataInput> is) ;

  /**
   * Constructor for the MultiTrie2 object
   *
   * @param forward set to <tt>true</tt> if the elements should be read left to
   *          right
   */
  MultiTrie2(bool forward);

  /**
   * Return the element that is stored in a cell associated with the given key.
   *
   * @param key the key to the cell holding the desired element
   * @return the element
   */
  std::shared_ptr<std::wstring>
  getFully(std::shared_ptr<std::wstring> key) override;

  /**
   * Return the element that is stored as last on a path belonging to the given
   * key.
   *
   * @param key the key associated with the desired element
   * @return the element that is stored as last on a path
   */
  std::shared_ptr<std::wstring>
  getLastOnPath(std::shared_ptr<std::wstring> key) override;

  /**
   * Write this data structure to the given output stream.
   *
   * @param os the output stream
   * @exception IOException if an I/O error occurs
   */
  void store(std::shared_ptr<DataOutput> os)  override;

  /**
   * Add an element to this structure consisting of the given key and patch
   * command.
   * <p>
   * This method will return without executing if the <tt>cmd</tt>
   * parameter's length is 0.
   *
   * @param key the key
   * @param cmd the patch command
   */
  void add(std::shared_ptr<std::wstring> key,
           std::shared_ptr<std::wstring> cmd) override;

  /**
   * Break the given patch command into its constituent pieces. The pieces are
   * delimited by NOOP commands.
   *
   * @param cmd the patch command
   * @return an array containing the pieces of the command
   */
  virtual std::deque<std::shared_ptr<std::wstring>>
  decompose(std::shared_ptr<std::wstring> cmd);

  /**
   * Remove empty rows from the given Trie and return the newly reduced Trie.
   *
   * @param by the Trie to reduce
   * @return the newly reduced Trie
   */
  std::shared_ptr<Trie> reduce(std::shared_ptr<Reduce> by) override;

private:
  bool cannotFollow(wchar_t after, wchar_t goes);

  std::shared_ptr<std::wstring> skip(std::shared_ptr<std::wstring> in_,
                                     int count);

  int dashEven(std::shared_ptr<std::wstring> in_, int from);

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") private int
  // lengthPP(std::wstring cmd)
  int lengthPP(std::shared_ptr<std::wstring> cmd);

protected:
  std::shared_ptr<MultiTrie2> shared_from_this()
  {
    return std::static_pointer_cast<MultiTrie2>(MultiTrie::shared_from_this());
  }
};

} // namespace org::egothor::stemmer
