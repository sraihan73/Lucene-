#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class MergeInfo;
}

namespace org::apache::lucene::store
{
class FlushInfo;
}

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
 * IOContext holds additional details on the merge/search context. A IOContext
 * object can never be initialized as null as passed as a parameter to either
 * {@link org.apache.lucene.store.Directory#openInput(std::wstring, IOContext)} or
 * {@link org.apache.lucene.store.Directory#createOutput(std::wstring, IOContext)}
 */
class IOContext : public std::enable_shared_from_this<IOContext>
{
  GET_CLASS_NAME(IOContext)

  /**
   * Context is a enumerator which specifies the context in which the Directory
   * is being used for.
   */
public:
  enum class Context { GET_CLASS_NAME(Context) MERGE, READ, FLUSH, DEFAULT };

  /**
   * An object of a enumerator Context type
   */
public:
  const Context context;

  const std::shared_ptr<MergeInfo> mergeInfo;

  const std::shared_ptr<FlushInfo> flushInfo;

  const bool readOnce;

  static const std::shared_ptr<IOContext> DEFAULT;

  static const std::shared_ptr<IOContext> READONCE;

  static const std::shared_ptr<IOContext> READ;

  IOContext();

  IOContext(std::shared_ptr<FlushInfo> flushInfo);

  IOContext(Context context);

private:
  IOContext(bool readOnce);

public:
  IOContext(std::shared_ptr<MergeInfo> mergeInfo);

private:
  IOContext(Context context, std::shared_ptr<MergeInfo> mergeInfo);

  /**
   * This constructor is used to initialize a {@link IOContext} instance with a
   * new value for the readOnce variable.
   * @param ctxt {@link IOContext} object whose information is used to create
   * the new instance except the readOnce variable.
   * @param readOnce The new {@link IOContext} object will use this value for
   * readOnce.
   */
public:
  IOContext(std::shared_ptr<IOContext> ctxt, bool readOnce);

  virtual int hashCode();

  bool equals(std::any obj) override;

  virtual std::wstring toString();
};
} // namespace org::apache::lucene::store