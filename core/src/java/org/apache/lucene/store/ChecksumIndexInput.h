#pragma once
#include "stringhelper.h"
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
 * Extension of IndexInput, computing checksum as it goes.
 * Callers can retrieve the checksum via {@link #getChecksum()}.
 */
class ChecksumIndexInput : public IndexInput
{
  GET_CLASS_NAME(ChecksumIndexInput)

  /** resourceDescription should be a non-null, opaque string
   *  describing this resource; it's returned from
   *  {@link #toString}. */
protected:
  ChecksumIndexInput(const std::wstring &resourceDescription);

  /** Returns the current checksum value */
public:
  virtual int64_t getChecksum() = 0;

  /**
   * {@inheritDoc}
   *
   * {@link ChecksumIndexInput} can only seek forward and seeks are expensive
   * since they imply to read bytes in-between the current position and the
   * target position in order to update the checksum.
   */
  void seek(int64_t pos)  override;

protected:
  std::shared_ptr<ChecksumIndexInput> shared_from_this()
  {
    return std::static_pointer_cast<ChecksumIndexInput>(
        IndexInput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
