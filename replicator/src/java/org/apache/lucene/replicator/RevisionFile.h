#pragma once
#include "stringhelper.h"
#include <any>
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
namespace org::apache::lucene::replicator
{

/**
 * Describes a file in a {@link Revision}. A file has a source, which allows a
 * single revision to contain files from multiple sources (e.g. multiple
 * indexes).
 *
 * @lucene.experimental
 */
class RevisionFile : public std::enable_shared_from_this<RevisionFile>
{
  GET_CLASS_NAME(RevisionFile)

  /** The name of the file. */
public:
  const std::wstring fileName;

  /** The size of the file denoted by {@link #fileName}. */
  int64_t size = -1;

  /** Constructor with the given file name. */
  RevisionFile(const std::wstring &fileName);

  bool equals(std::any obj) override;

  virtual int hashCode();

  virtual std::wstring toString();
};

} // namespace org::apache::lucene::replicator
