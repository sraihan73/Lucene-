#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::store
{

/**
 * A {@link DataOutput} wrapping a plain {@link OutputStream}.
 */
class OutputStreamDataOutput : public DataOutput
{
  GET_CLASS_NAME(OutputStreamDataOutput)
private:
  const std::shared_ptr<OutputStream> os;

public:
  OutputStreamDataOutput(std::shared_ptr<OutputStream> os);

  void writeByte(char b)  override;

  void writeBytes(std::deque<char> &b, int offset,
                  int length)  override;

  virtual ~OutputStreamDataOutput();

protected:
  std::shared_ptr<OutputStreamDataOutput> shared_from_this()
  {
    return std::static_pointer_cast<OutputStreamDataOutput>(
        DataOutput::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
