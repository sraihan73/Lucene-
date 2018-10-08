#pragma once
#include "stringhelper.h"
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
 * DocValues types. Note that DocValues is strongly typed, so a
 * field cannot have different types across different documents.
 */
enum class DocValuesType {
  GET_CLASS_NAME(DocValuesType)
  /**
   * No doc values for this field.
   */
  NONE,
  /**
   * A per-document Number
   */
  NUMERIC,
  /**
   * A per-document byte[].  Values may be larger than
   * 32766 bytes, but different codecs may enforce their own limits.
   */
  BINARY,
  /**
   * A pre-sorted byte[]. Fields with this type only store distinct byte values
   * and store an additional offset pointer per document to dereference the
   * shared byte[]. The stored byte[] is presorted and allows access via
   * document id, ordinal and by-value.  Values must be {@code <= 32766} bytes.
   */
  SORTED,
  /**
   * A pre-sorted Number[]. Fields with this type store numeric values in sorted
   * order according to {@link Long#compare(long, long)}.
   */
  SORTED_NUMERIC,
  /**
   * A pre-sorted Set&lt;byte[]&gt;. Fields with this type only store distinct
   * byte values and store additional offset pointers per document to
   * dereference the shared byte[]s. The stored byte[] is presorted and allows
   * access via document id, ordinal and by-value.  Values must be {@code <=
   * 32766} bytes.
   */
  SORTED_SET,
};

} // namespace org::apache::lucene::index
