#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::analysis::cn::smart::hhmm
{

/**
 * <p>
 * SmartChineseAnalyzer abstract dictionary implementation.
 * </p>
 * <p>
 * Contains methods for dealing with GB2312 encoding.
 * </p>
 * @lucene.experimental
 */
class AbstractDictionary
    : public std::enable_shared_from_this<AbstractDictionary>
{
  GET_CLASS_NAME(AbstractDictionary)
  /**
   * First Chinese Character in GB2312 (15 * 94)
   * Characters in GB2312 are arranged in a grid of 94 * 94, 0-14 are unassigned
   * or punctuation.
   */
public:
  static constexpr int GB2312_FIRST_CHAR = 1410;

  /**
   * Last Chinese Character in GB2312 (87 * 94).
   * Characters in GB2312 are arranged in a grid of 94 * 94, 88-94 are
   * unassigned.
   */
  static constexpr int GB2312_CHAR_NUM = 87 * 94;

  /**
   * Dictionary data contains 6768 Chinese characters with frequency statistics.
   */
  static constexpr int CHAR_NUM_IN_FILE = 6768;

  // =====================================================
  // code +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F
  // B0A0 啊 阿 埃 挨 哎 唉 哀 皑 癌 蔼 矮 艾 碍 爱 隘
  // B0B0 鞍 氨 安 俺 按 暗 岸 胺 案 肮 昂 盎 凹 敖 熬 翱
  // B0C0 袄 傲 奥 懊 澳 芭 捌 扒 叭 吧 笆 八 疤 巴 拔 跋
  // B0D0 靶 把 耙 坝 霸 罢 爸 白 柏 百 摆 佰 败 拜 稗 斑
  // B0E0 班 搬 扳 般 颁 板 版 扮 拌 伴 瓣 半 办 绊 邦 帮
  // B0F0 梆 榜 膀 绑 棒 磅 蚌 镑 傍 谤 苞 胞 包 褒 剥
  // =====================================================
  //
  // GB2312 character set：
  // 01 94 Symbols
  // 02 72 Numbers
  // 03 94 Latin
  // 04 83 Kana
  // 05 86 Katakana
  // 06 48 Greek
  // 07 66 Cyrillic
  // 08 63 Phonetic Symbols
  // 09 76 Drawing Symbols
  // 10-15 Unassigned
  // 16-55 3755 Plane 1, in pinyin order
  // 56-87 3008 Plane 2, in radical/stroke order
  // 88-94 Unassigned
  // ======================================================

  /**
   * <p>
   * Transcode from GB2312 ID to Unicode
   * </p>
   * <p>
   * GB2312 is divided into a 94 * 94 grid, containing 7445 characters
   * consisting of 6763 Chinese characters and 682 symbols. Some regions are
   * unassigned (reserved).
   * </p>
   *
   * @param ccid GB2312 id
   * @return unicode std::wstring
   */
  virtual std::wstring getCCByGB2312Id(int ccid);

  /**
   * Transcode from Unicode to GB2312
   *
   * @param ch input character in Unicode, or character in Basic Latin range.
   * @return position in GB2312
   */
  virtual short getGB2312Id(wchar_t ch);

  /**
   * 32-bit FNV Hash Function
   *
   * @param c input character
   * @return hashcode
   */
  virtual int64_t hash1(wchar_t c);

  /**
   * 32-bit FNV Hash Function
   *
   * @param carray character array
   * @return hashcode
   */
  virtual int64_t hash1(std::deque<wchar_t> &carray);

  /**
   * djb2 hash algorithm，this algorithm (k=33) was first reported by dan
   * bernstein many years ago in comp.lang.c. another version of this algorithm
   * (now favored by bernstein) uses xor: hash(i) = hash(i - 1) * 33 ^ str[i];
   * the magic of number 33 (why it works better than many other constants,
   * prime or not) has never been adequately explained.
   *
   * @param c character
   * @return hashcode
   */
  virtual int hash2(wchar_t c);

  /**
   * djb2 hash algorithm，this algorithm (k=33) was first reported by dan
   * bernstein many years ago in comp.lang.c. another version of this algorithm
   * (now favored by bernstein) uses xor: hash(i) = hash(i - 1) * 33 ^ str[i];
   * the magic of number 33 (why it works better than many other constants,
   * prime or not) has never been adequately explained.
   *
   * @param carray character array
   * @return hashcode
   */
  virtual int hash2(std::deque<wchar_t> &carray);
};

} // #include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/
