#pragma once
#include "AbstractDictionary.h"
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
 * SmartChineseAnalyzer Bigram dictionary.
 * @lucene.experimental
 */
class BigramDictionary : public AbstractDictionary
{
  GET_CLASS_NAME(BigramDictionary)

private:
  BigramDictionary();

public:
  static constexpr wchar_t WORD_SEGMENT_CHAR = L'@';

private:
  static std::shared_ptr<BigramDictionary> singleInstance;

public:
  static constexpr int PRIME_BIGRAM_LENGTH = 402137;

  /*
   * The word associations are stored as FNV1 hashcodes, which have a small
   * probability of collision, but save memory.
   */
private:
  std::deque<int64_t> bigramHashTable;

  std::deque<int> frequencyTable;

  int max = 0;

  int repeat = 0;

  // static Logger log = Logger.getLogger(BigramDictionary.class);

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  static std::shared_ptr<BigramDictionary> getInstance();

private:
  bool loadFromObj(std::shared_ptr<Path> serialObj);

  void loadFromInputStream(
      std::shared_ptr<InputStream>
          serialObjectInputStream) ;

  void saveToObj(std::shared_ptr<Path> serialObj) ;

  void load() ;

  void load(const std::wstring &dictRoot) ;

  /**
   * Load the datafile into this BigramDictionary
   *
   * @param dctFilePath path to the Bigramdictionary (bigramdict.dct)
   * @throws IOException If there is a low-level I/O error
   */
public:
  virtual void loadFromFile(const std::wstring &dctFilePath) ;

private:
  int getAvaliableIndex(int64_t hashId, std::deque<wchar_t> &carray);

  /*
   * lookup the index into the frequency array.
   */
  int getBigramItemIndex(std::deque<wchar_t> &carray);

public:
  virtual int getFrequency(std::deque<wchar_t> &carray);

protected:
  std::shared_ptr<BigramDictionary> shared_from_this()
  {
    return std::static_pointer_cast<BigramDictionary>(
        AbstractDictionary::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/
