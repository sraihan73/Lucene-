#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

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
namespace org::apache::lucene::store
{

/**
 * <p>A MergeInfo provides information required for a MERGE context.
 *  It is used as part of an {@link IOContext} in case of MERGE context.</p>
 */

class MergeInfo : public std::enable_shared_from_this<MergeInfo>
{
  GET_CLASS_NAME(MergeInfo)

public:
  const int totalMaxDoc;

  const int64_t estimatedMergeBytes;

  const bool isExternal;

  const int mergeMaxNumSegments;

  /**
   * <p>Creates a new {@link MergeInfo} instance from
   * the values required for a MERGE {@link IOContext} context.
   *
   * These values are only estimates and are not the actual values.
   *
   */

  MergeInfo(int totalMaxDoc, int64_t estimatedMergeBytes, bool isExternal,
            int mergeMaxNumSegments);

  virtual int hashCode();

  bool equals(std::any obj) override;

  virtual std::wstring toString();
};

} // namespace org::apache::lucene::store
