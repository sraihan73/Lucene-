#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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

/** Abstract base class for output to a file in a Directory.  A random-access
 * output stream.  Used for all Lucene index output operations.

 * <p>{@code IndexOutput} may only be used from one thread, because it is not
 * thread safe (it keeps internal state like file position).

 * @see Directory
 * @see IndexInput
 */
class IndexOutput : public DataOutput
{
  GET_CLASS_NAME(IndexOutput)

  /** Full description of this output, e.g. which class such as {@code
   * FSIndexOutput}, and the full path to the file */
private:
  const std::wstring resourceDescription;

  /** Just the name part from {@code resourceDescription} */
  const std::wstring name;

  /** Sole constructor.  resourceDescription should be non-null, opaque string
   *  describing this resource; it's returned from {@link #toString}. */
protected:
  IndexOutput(const std::wstring &resourceDescription,
              const std::wstring &name);

  /** Returns the name used to create this {@code IndexOutput}.  This is
   * especially useful when using
   * {@link Directory#createTempOutput}. */
  // TODO: can we somehow use this as the default resource description or
  // something?
public:
  virtual std::wstring getName();

  /** Closes this stream to further operations. */
  void close() = 0;
  override

      /** Returns the current position in this file, where the next write will
       * occur.
       */
      virtual int64_t
      getFilePointer() = 0;

  /** Returns the current checksum of bytes written so far */
  virtual int64_t getChecksum() = 0;

  virtual std::wstring toString();

protected:
  std::shared_ptr<IndexOutput> shared_from_this()
  {
    return std::static_pointer_cast<IndexOutput>(
        DataOutput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
