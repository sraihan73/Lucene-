#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/LockFactory.h"

#include  "core/src/java/org/apache/lucene/store/Lock.h"
#include  "core/src/java/org/apache/lucene/store/AlreadyClosedException.h"

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
 * Base implementation for a concrete {@link Directory} that uses a {@link
 * LockFactory} for locking.
 * @lucene.experimental
 */
class BaseDirectory : public Directory
{
  GET_CLASS_NAME(BaseDirectory)

protected:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile protected bool isOpen = true;
  bool isOpen = true;

  /** Holds the LockFactory instance (implements locking for
   * this Directory instance). */
  const std::shared_ptr<LockFactory> lockFactory;

  /** Sole constructor. */
  BaseDirectory(std::shared_ptr<LockFactory> lockFactory);

public:
  std::shared_ptr<Lock>
  obtainLock(const std::wstring &name)  override final;

protected:
  void ensureOpen()  override final;

public:
  virtual std::wstring toString();

protected:
  std::shared_ptr<BaseDirectory> shared_from_this()
  {
    return std::static_pointer_cast<BaseDirectory>(
        Directory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
