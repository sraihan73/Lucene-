#pragma once
#include "stringhelper.h"
#include "tangible_filesystem.h"
#include <memory>
#include <string>
#include <typeinfo>
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
namespace org::apache::lucene::analysis::ko::util
{

class BinaryDictionaryWriter
    : public std::enable_shared_from_this<BinaryDictionaryWriter>
{
  GET_CLASS_NAME(BinaryDictionaryWriter)
protected:
  const std::type_info implClazz;
  std::shared_ptr<ByteBuffer> buffer;

private:
  int targetMapEndOffset = 0, lastWordId = -1, lastSourceId = -1;
  std::deque<int> targetMap = std::deque<int>(8192);
  std::deque<int> targetMapOffsets = std::deque<int>(8192);
  const std::deque<std::wstring> posDict = std::deque<std::wstring>();

public:
  BinaryDictionaryWriter(std::type_info implClazz, int size);

  /**
   * put the entry in map_obj
   *
   * mecab-ko-dic features
   *
   * 0   - surface
   * 1   - left cost
   * 2   - right cost
   * 3   - word cost
   * 4   - part of speech0+part of speech1+...
   * 5   - semantic class
   * 6   - T if the last character of the surface form has a coda, F otherwise
   * 7   - reading
   * 8   - POS type (*, Compound, Inflect, Preanalysis)
   * 9   - left POS
   * 10  - right POS
   * 11  - expression
   *
   * @return current position of buffer, which will be wordId of next entry
   */
  virtual int put(std::deque<std::wstring> &entry);

private:
  void writeString(const std::wstring &s);

public:
  virtual void addMapping(int sourceId, int wordId);

protected:
  std::wstring getBaseFileName(const std::wstring &baseDir);

  /**
   * Write dictionary in file
   * @throws IOException if an I/O error occurs writing the dictionary files
   */
public:
  virtual void write(const std::wstring &baseDir) ;

protected:
  virtual void writeTargetMap(const std::wstring &filename) ;

  virtual void writePosDict(const std::wstring &filename) ;

  virtual void writeDictionary(const std::wstring &filename) ;
};

} // namespace org::apache::lucene::analysis::ko::util
