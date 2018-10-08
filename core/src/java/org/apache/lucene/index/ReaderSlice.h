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
namespace org::apache::lucene::index
{

/**
 * Subreader slice from a parent composite reader.
 *
 * @lucene.internal
 */
class ReaderSlice final : public std::enable_shared_from_this<ReaderSlice>
{
  GET_CLASS_NAME(ReaderSlice)

  /** Zero-length {@code ReaderSlice} array. */
public:
  static std::deque<std::shared_ptr<ReaderSlice>> const EMPTY_ARRAY;

  /** Document ID this slice starts from. */
  const int start;

  /** Number of documents in this slice. */
  const int length;

  /** Sub-reader index for this slice. */
  const int readerIndex;

  /** Sole constructor. */
  ReaderSlice(int start, int length, int readerIndex);

  virtual std::wstring toString();
};
} // #include  "core/src/java/org/apache/lucene/index/
