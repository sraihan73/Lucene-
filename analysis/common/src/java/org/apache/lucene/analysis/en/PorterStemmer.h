#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
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
namespace org::apache::lucene::analysis::en
{

/*

   Porter stemmer in Java. The original paper is in

       Porter, 1980, An algorithm for suffix stripping, Program, Vol. 14,
       no. 3, pp 130-137,

   See also http://www.tartarus.org/~martin/PorterStemmer/index.html

   Bug 1 (reported by Gonzalo Parra 16/10/99) fixed as marked below.
   Tthe words 'aed', 'eed', 'oed' leave k at 'a' for step 3, and b[k-1]
   is then out outside the bounds of b.

   Similarly,

   Bug 2 (reported by Steve Dyrdahl 22/2/00) fixed as marked below.
   'ion' by itself leaves j = -1 in the test for 'ion' in step 5, and
   b[j] is then outside the bounds of b.

   Release 3.

   [ This version is derived from Release 3, modified by Brian Goetz to
     optimize for fewer object creations.  ]

*/

/**
 *
 * Stemmer, implementing the Porter Stemming Algorithm
 *
 * The Stemmer class transforms a word into its root form.  The input
 * word can be provided a character at time (by calling add()), or at once
 * by calling one of the various stem(something) methods.
 */

class PorterStemmer : public std::enable_shared_from_this<PorterStemmer>
{
  GET_CLASS_NAME(PorterStemmer)
private:
  std::deque<wchar_t> b;
  int i = 0, j = 0, k = 0, k0 = 0;
  bool dirty = false;
  static constexpr int INITIAL_SIZE = 50;

public:
  PorterStemmer();

  /**
   * reset() resets the stemmer so it can stem another word.  If you invoke
   * the stemmer by calling add(char) and then stem(), you must call reset()
   * before starting another word.
   */
  virtual void reset();

  /**
   * Add a character to the word being stemmed.  When you are finished
   * adding characters, you can call stem(void) to process the word.
   */
  virtual void add(wchar_t ch);

  /**
   * After a word has been stemmed, it can be retrieved by toString(),
   * or a reference to the internal buffer can be retrieved by getResultBuffer
   * and getResultLength (which is generally more efficient.)
   */
  virtual std::wstring toString();

  /**
   * Returns the length of the word resulting from the stemming process.
   */
  virtual int getResultLength();

  /**
   * Returns a reference to a character buffer containing the results of
   * the stemming process.  You also need to consult getResultLength()
   * to determine the length of the result.
   */
  virtual std::deque<wchar_t> getResultBuffer();

  /* cons(i) is true <=> b[i] is a consonant. */

private:
  bool cons(int i);

  /* m() measures the number of consonant sequences between k0 and j. if c is
     a consonant sequence and v a vowel sequence, and <..> indicates arbitrary
     presence,

          <c><v>       gives 0
          <c>vc<v>     gives 1
          <c>vcvc<v>   gives 2
          <c>vcvcvc<v> gives 3
          ....
  */

  int m();

  /* vowelinstem() is true <=> k0,...j contains a vowel */

  bool vowelinstem();

  /* doublec(j) is true <=> j,(j-1) contain a double consonant. */

  bool doublec(int j);

  /* cvc(i) is true <=> i-2,i-1,i has the form consonant - vowel - consonant
     and also if the second c is not w,x or y. this is used when trying to
     restore an e at the end of a short word. e.g.

          cav(e), lov(e), hop(e), crim(e), but
          snow, box, tray.

  */

  bool cvc(int i);

  bool ends(const std::wstring &s);

  /* setto(s) sets (j+1),...k to the characters in the string s, readjusting
     k. */

public:
  virtual void setto(const std::wstring &s);

  /* r(s) is used further down. */

  virtual void r(const std::wstring &s);

  /* step1() gets rid of plurals and -ed or -ing. e.g.

           caresses  ->  caress
           ponies    ->  poni
           ties      ->  ti
           caress    ->  caress
           cats      ->  cat

           feed      ->  feed
           agreed    ->  agree
           disabled  ->  disable

           matting   ->  mat
           mating    ->  mate
           meeting   ->  meet
           milling   ->  mill
           messing   ->  mess

           meetings  ->  meet

  */

private:
  void step1();

  /* step2() turns terminal y to i when there is another vowel in the stem. */

  void step2();

  /* step3() maps double suffices to single ones. so -ization ( = -ize plus
     -ation) maps to -ize etc. note that the string before the suffix must give
     m() > 0. */

  void step3();

  /* step4() deals with -ic-, -full, -ness etc. similar strategy to step3. */

  void step4();

  /* step5() takes off -ant, -ence etc., in context <c>vcvc<v>. */

  void step5();

  /* step6() removes a final -e if m() > 1. */

  void step6();

  /**
   * Stem a word provided as a std::wstring.  Returns the result as a std::wstring.
   */
public:
  virtual std::wstring stem(const std::wstring &s);

  /** Stem a word contained in a char[].  Returns true if the stemming process
   * resulted in a word different from the input.  You can retrieve the
   * result with getResultLength()/getResultBuffer() or toString().
   */
  virtual bool stem(std::deque<wchar_t> &word);

  /** Stem a word contained in a portion of a char[] array.  Returns
   * true if the stemming process resulted in a word different from
   * the input.  You can retrieve the result with
   * getResultLength()/getResultBuffer() or toString().
   */
  virtual bool stem(std::deque<wchar_t> &wordBuffer, int offset, int wordLen);

  /** Stem a word contained in a leading portion of a char[] array.
   * Returns true if the stemming process resulted in a word different
   * from the input.  You can retrieve the result with
   * getResultLength()/getResultBuffer() or toString().
   */
  virtual bool stem(std::deque<wchar_t> &word, int wordLen);

  /** Stem the word placed into the Stemmer buffer through calls to add().
   * Returns true if the stemming process resulted in a word different
   * from the input.  You can retrieve the result with
   * getResultLength()/getResultBuffer() or toString().
   */
  virtual bool stem();

  virtual bool stem(int i0);

  /* Test program for demonstrating the Stemmer.  It reads a file and
   * stems each word, writing the result to standard out.
   * Usage: Stemmer file-name
  public static void main(std::wstring[] args) {
    PorterStemmer s = new PorterStemmer();

    for (int i = 0; i < args.length; i++) {
      try {
        InputStream in = new FileInputStream(args[i]);
        byte[] buffer = new byte[1024];
        int bufferLen, offset, ch;

        bufferLen = in.read(buffer);
        offset = 0;
        s.reset();

        while(true) {
          if (offset < bufferLen)
            ch = buffer[offset++];
          else {
            bufferLen = in.read(buffer);
            offset = 0;
            if (bufferLen < 0)
              ch = -1;
            else
              ch = buffer[offset++];
          }

          if (Character.isLetter((char) ch)) {
            s.add(Character.toLowerCase((char) ch));
          }
          else {
             s.stem();
             System.out.print(s.toString());
             s.reset();
             if (ch < 0)
               break;
             else {
               System.out.print((char) ch);
             }
           }
        }

        in.close();
      }
      catch (IOException e) {
        System.out.println("error reading " + args[i]);
      }
    }
  }*/
};

} // #include  "core/src/java/org/apache/lucene/analysis/en/
