#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
 * The Diff object generates a patch string.
 * <p>
 * A patch string is actually a command to a stemmer telling it how to reduce a
 * word to its root. For example, to reduce the word teacher to its root teach
 * the patch string Db would be generated. This command tells the stemmer to
 * delete the last 2 characters from the word teacher to reach the stem (the
 * patch commands are applied starting from the last character in order to save
 */
class Diff : public std::enable_shared_from_this<Diff>
{
  GET_CLASS_NAME(Diff)
public:
  int sizex = 0;
  int sizey = 0;
  std::deque<std::deque<int>> net;
  std::deque<std::deque<int>> way;

  int INSERT = 0;
  int DELETE = 0;
  int REPLACE = 0;
  int NOOP = 0;

  /**
   * Constructor for the Diff object.
   */
  Diff();

  /**
   * Constructor for the Diff object
   *
   * @param ins Description of the Parameter
   * @param del Description of the Parameter
   * @param rep Description of the Parameter
   * @param noop Description of the Parameter
   */
  Diff(int ins, int del, int rep, int noop);

  /**
   * Apply the given patch string <tt>diff</tt> to the given string <tt>
   * dest</tt>.
   *
   * @param dest Destination string
   * @param diff Patch string
   */
  static void apply(std::shared_ptr<StringBuilder> dest,
                    std::shared_ptr<std::wstring> diff);

  /**
   * Construct a patch string that transforms a to b.
   *
   * @param a std::wstring 1st string
   * @param b std::wstring 2nd string
   * @return std::wstring
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::wstring exec(const std::wstring &a, const std::wstring &b);
};

} // #include  "core/src/java/org/egothor/stemmer/
