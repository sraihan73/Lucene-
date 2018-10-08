#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::egothor::stemmer
{
class Row;
}

namespace org::egothor::stemmer
{
class Reduce;
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
 * A Trie is used to store a dictionary of words and their stems.
 * <p>
 * Actually, what is stored are words with their respective patch commands. A
 * trie can be termed forward (keys read from left to right) or backward (keys
 * read from right to left). This property will vary depending on the language
 * for which a Trie is constructed.
 */
class Trie : public std::enable_shared_from_this<Trie>
{
  GET_CLASS_NAME(Trie)
public:
  std::deque<std::shared_ptr<Row>> rows = std::deque<std::shared_ptr<Row>>();
  std::deque<std::shared_ptr<std::wstring>> cmds =
      std::deque<std::shared_ptr<std::wstring>>();
  int root = 0;

  bool forward = false;

  /**
   * Constructor for the Trie object.
   *
   * @param is the input stream
   * @exception IOException if an I/O error occurs
   */
  Trie(std::shared_ptr<DataInput> is) ;

  /**
   * Constructor for the Trie object.
   *
   * @param forward set to <tt>true</tt>
   */
  Trie(bool forward);

  /**
   * Constructor for the Trie object.
   *
   * @param forward <tt>true</tt> if read left to right, <tt>false</tt> if read
   *          right to left
   * @param root index of the row that is the root node
   * @param cmds the patch commands to store
   * @param rows a Vector of Vectors. Each inner Vector is a node of this Trie
   */
  Trie(bool forward, int root, std::deque<std::shared_ptr<std::wstring>> &cmds,
       std::deque<std::shared_ptr<Row>> &rows);

  /**
   * Gets the all attribute of the Trie object
   *
   * @param key Description of the Parameter
   * @return The all value
   */
  virtual std::deque<std::shared_ptr<std::wstring>>
  getAll(std::shared_ptr<std::wstring> key);

  /**
   * Return the number of cells in this Trie object.
   *
   * @return the number of cells
   */
  virtual int getCells();

  /**
   * Gets the cellsPnt attribute of the Trie object
   *
   * @return The cellsPnt value
   */
  virtual int getCellsPnt();

  /**
   * Gets the cellsVal attribute of the Trie object
   *
   * @return The cellsVal value
   */
  virtual int getCellsVal();

  /**
   * Return the element that is stored in a cell associated with the given key.
   *
   * @param key the key
   * @return the associated element
   */
  virtual std::shared_ptr<std::wstring>
  getFully(std::shared_ptr<std::wstring> key);

  /**
   * Return the element that is stored as last on a path associated with the
   * given key.
   *
   * @param key the key associated with the desired element
   * @return the last on path element
   */
  virtual std::shared_ptr<std::wstring>
  getLastOnPath(std::shared_ptr<std::wstring> key);

  /**
   * Return the Row at the given index.
   *
   * @param index the index containing the desired Row
   * @return the Row
   */
private:
  std::shared_ptr<Row> getRow(int index);

  /**
   * Write this Trie to the given output stream.
   *
   * @param os the output stream
   * @exception IOException if an I/O error occurs
   */
public:
  virtual void store(std::shared_ptr<DataOutput> os) ;

  /**
   * Add the given key associated with the given patch command. If either
   * parameter is null this method will return without executing.
   *
   * @param key the key
   * @param cmd the patch command
   */
  virtual void add(std::shared_ptr<std::wstring> key,
                   std::shared_ptr<std::wstring> cmd);

  /**
   * Remove empty rows from the given Trie and return the newly reduced Trie.
   *
   * @param by the Trie to reduce
   * @return the newly reduced Trie
   */
  virtual std::shared_ptr<Trie> reduce(std::shared_ptr<Reduce> by);

  /** writes debugging info to the printstream */
  virtual void printInfo(std::shared_ptr<PrintStream> out,
                         std::shared_ptr<std::wstring> prefix);

  /**
   * This class is part of the Egothor Project
   */
public:
  class StrEnum : public std::enable_shared_from_this<StrEnum>
  {
    GET_CLASS_NAME(StrEnum)
  private:
    std::shared_ptr<Trie> outerInstance;

  public:
    std::shared_ptr<std::wstring> s;
    int from = 0;
    int by = 0;

    /**
     * Constructor for the StrEnum object
     *
     * @param s Description of the Parameter
     * @param up Description of the Parameter
     */
    StrEnum(std::shared_ptr<Trie> outerInstance,
            std::shared_ptr<std::wstring> s, bool up);

    virtual int length();

    virtual wchar_t next();
  };
};

} // namespace org::egothor::stemmer
