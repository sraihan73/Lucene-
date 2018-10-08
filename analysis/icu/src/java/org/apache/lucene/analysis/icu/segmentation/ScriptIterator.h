#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

/*
 * Copyright (C) 1999-2010, International Business Machines
 * Corporation and others.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * provided that the above copyright notice(s) and this permission notice appear
 * in all copies of the Software and that both the above copyright notice(s) and
 * this permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS NOTICE BE
 * LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder shall not
 * be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization of the
 * copyright holder.
 */
namespace org::apache::lucene::analysis::icu::segmentation
{

/**
 * An iterator that locates ISO 15924 script boundaries in text.
 * <p>
 * This is not the same as simply looking at the Unicode block, or even the
 * Script property. Some characters are 'common' across multiple scripts, and
 * some 'inherit' the script value of text surrounding them.
 * <p>
 * This is similar to ICU (internal-only) UScriptRun, with the following
 * differences:
 * <ul>
 *  <li>Doesn't attempt to match paired punctuation. For tokenization purposes,
 * this is not necessary. It's also quite expensive. <li>Non-spacing marks
 * inherit the script of their base character, following recommendations from
 * UTR #24.
 * </ul>
 * @lucene.experimental
 */
class ScriptIterator final : public std::enable_shared_from_this<ScriptIterator>
{
  GET_CLASS_NAME(ScriptIterator)
private:
  std::deque<wchar_t> text;
  int start = 0;
  int limit = 0;
  int index = 0;

  int scriptStart = 0;
  int scriptLimit = 0;
  int scriptCode = 0;

  const bool combineCJ;

  /**
   * @param combineCJ if true: Han,Hiragana,Katakana will all return as {@link
   * UScript#JAPANESE}
   */
public:
  ScriptIterator(bool combineCJ);

  /**
   * Get the start of this script run
   *
   * @return start position of script run
   */
  int getScriptStart();

  /**
   * Get the index of the first character after the end of this script run
   *
   * @return position of the first character after this script run
   */
  int getScriptLimit();

  /**
   * Get the UScript script code for this script run
   *
   * @return code for the script of the current run
   */
  int getScriptCode();

  /**
   * Iterates to the next script run, returning true if one exists.
   *
   * @return true if there is another script run, false otherwise.
   */
  bool next();

  /** Determine if two scripts are compatible. */
private:
  static bool isSameScript(int scriptOne, int scriptTwo);

  /**
   * Set a new region of text to be examined by this iterator
   *
   * @param text text buffer to examine
   * @param start offset into buffer
   * @param length maximum length to examine
   */
public:
  void setText(std::deque<wchar_t> &text, int start, int length);

  /** linear fast-path for basic latin case */
private:
  static std::deque<int> const basicLatin;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static ScriptIterator::StaticConstructor staticConstructor;

  /** fast version of UScript.getScript(). Basic Latin is an array lookup */
  int getScript(int codepoint);
};

} // #include  "core/src/java/org/apache/lucene/analysis/icu/segmentation/
