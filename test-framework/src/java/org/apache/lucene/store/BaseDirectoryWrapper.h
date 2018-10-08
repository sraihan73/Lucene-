#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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
 * Calls check index on close.
 */
// do NOT make any methods in this class synchronized, volatile
// do NOT import anything from the concurrency package.
// no randoms, no nothing.
class BaseDirectoryWrapper : public FilterDirectory
{
  GET_CLASS_NAME(BaseDirectoryWrapper)

private:
  bool checkIndexOnClose = true;
  bool crossCheckTermVectorsOnClose = true;

protected:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: protected volatile bool isOpen = true;
  // C++ NOTE: Fields cannot have the same name as methods:
  bool isOpen_ = true;

  BaseDirectoryWrapper(std::shared_ptr<Directory> delegate_);

public:
  virtual ~BaseDirectoryWrapper();

  virtual bool isOpen();

  /**
   * Set whether or not checkindex should be run
   * on close
   */
  virtual void setCheckIndexOnClose(bool value);

  virtual bool getCheckIndexOnClose();

  virtual void setCrossCheckTermVectorsOnClose(bool value);

  virtual bool getCrossCheckTermVectorsOnClose();

protected:
  std::shared_ptr<BaseDirectoryWrapper> shared_from_this()
  {
    return std::static_pointer_cast<BaseDirectoryWrapper>(
        FilterDirectory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
