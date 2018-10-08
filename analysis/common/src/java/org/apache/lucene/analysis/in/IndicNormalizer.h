#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::analysis::in_
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static Character.UnicodeBlock.*;
using namespace org::apache::lucene::analysis::util;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

/**
 * Normalizes the Unicode representation of text in Indian languages.
 * <p>
 * Follows guidelines from Unicode 5.2, chapter 6, South Asian Scripts I
 * and graphical decompositions from
 * http://ldc.upenn.edu/myl/IndianScriptsUnicode.html
 * </p>
 */
class IndicNormalizer : public std::enable_shared_from_this<IndicNormalizer>
{
  GET_CLASS_NAME(IndicNormalizer)

private:
  class ScriptData : public std::enable_shared_from_this<ScriptData>
  {
    GET_CLASS_NAME(ScriptData)
  public:
    const int flag;
    const int base;
    std::shared_ptr<BitSet> decompMask;

    ScriptData(int flag, int base);
  };

private:
  static const std::shared_ptr<IdentityHashMap<
      std::shared_ptr<Character::UnicodeBlock>, std::shared_ptr<ScriptData>>>
      scripts;

  static int flag(std::shared_ptr<Character::UnicodeBlock> ub);

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static IndicNormalizer::StaticConstructor staticConstructor;

  /**
   * Decompositions according to Unicode 5.2,
   * and http://ldc.upenn.edu/myl/IndianScriptsUnicode.html
   *
   * Most of these are not handled by unicode normalization anyway.
   *
   * The numbers here represent offsets into the respective codepages,
   * with -1 representing null and 0xFF representing zero-width joiner.
   *
   * the columns are: ch1, ch2, ch3, res, flags
   * ch1, ch2, and ch3 are the decomposition
   * res is the composition, and flags are the scripts to which it applies.
   */
  static std::deque<std::deque<int>> const decompositions;

  /**
   * Normalizes input text, and returns the new length.
   * The length will always be less than or equal to the existing length.
   *
   * @param text input text
   * @param len valid length
   * @return normalized length
   */
public:
  virtual int normalize(std::deque<wchar_t> &text, int len);

  /**
   * Compose into standard form any compositions in the decompositions table.
   */
private:
  int compose(int ch0, std::shared_ptr<Character::UnicodeBlock> block0,
              std::shared_ptr<ScriptData> sd, std::deque<wchar_t> &text,
              int pos, int len);
};

} // #include  "core/src/java/org/apache/lucene/analysis/in_/
