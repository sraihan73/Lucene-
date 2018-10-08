#pragma once
#include "Dictionary.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/IntsRef.h"

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
namespace org::apache::lucene::analysis::ja::dict
{

using IntsRef = org::apache::lucene::util::IntsRef;

/**
 * Base class for a binary-encoded in-memory dictionary.
 */
class BinaryDictionary : public std::enable_shared_from_this<BinaryDictionary>,
                         public Dictionary
{
  GET_CLASS_NAME(BinaryDictionary)

public:
  static const std::wstring DICT_FILENAME_SUFFIX;
  static const std::wstring TARGETMAP_FILENAME_SUFFIX;
  static const std::wstring POSDICT_FILENAME_SUFFIX;

  static const std::wstring DICT_HEADER;
  static const std::wstring TARGETMAP_HEADER;
  static const std::wstring POSDICT_HEADER;
  static constexpr int VERSION = 1;

private:
  const std::shared_ptr<ByteBuffer> buffer;
  std::deque<int> const targetMapOffsets, targetMap;
  std::deque<std::wstring> const posDict;
  std::deque<std::wstring> const inflTypeDict;
  std::deque<std::wstring> const inflFormDict;

protected:
  BinaryDictionary() ;

  std::shared_ptr<InputStream>
  getResource(const std::wstring &suffix) ;

  // util, reused by ConnectionCosts and CharacterDefinition
public:
  static std::shared_ptr<InputStream>
  getClassResource(std::type_info clazz,
                   const std::wstring &suffix) ;

  virtual void lookupWordIds(int sourceId, std::shared_ptr<IntsRef> ref);

  int getLeftId(int wordId) override;

  int getRightId(int wordId) override;

  int getWordCost(int wordId) override;

  std::wstring getBaseForm(int wordId, std::deque<wchar_t> &surfaceForm,
                           int off, int len) override;

  std::wstring getReading(int wordId, std::deque<wchar_t> &surface, int off,
                          int len) override;

  std::wstring getPartOfSpeech(int wordId) override;

  std::wstring getPronunciation(int wordId, std::deque<wchar_t> &surface,
                                int off, int len) override;

  std::wstring getInflectionType(int wordId) override;

  std::wstring getInflectionForm(int wordId) override;

private:
  static int baseFormOffset(int wordId);

  int readingOffset(int wordId);

  int pronunciationOffset(int wordId);

  bool hasBaseFormData(int wordId);

  bool hasReadingData(int wordId);

  bool hasPronunciationData(int wordId);

  std::wstring readString(int offset, int length, bool kana);

  /** flag that the entry has baseform data. otherwise it's not inflected (same
   * as surface form) */
public:
  static constexpr int HAS_BASEFORM = 1;
  /** flag that the entry has reading data. otherwise reading is surface form
   * converted to katakana */
  static constexpr int HAS_READING = 2;
  /** flag that the entry has pronunciation data. otherwise pronunciation is the
   * reading */
  static constexpr int HAS_PRONUNCIATION = 4;
};

} // #include  "core/src/java/org/apache/lucene/analysis/ja/dict/
