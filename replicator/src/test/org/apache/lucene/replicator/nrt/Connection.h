#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/DataInput.h"

#include  "core/src/java/org/apache/lucene/store/DataOutput.h"

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

namespace org::apache::lucene::replicator::nrt
{

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;

/** Simple point-to-point TCP connection */
class Connection : public std::enable_shared_from_this<Connection>
{
  GET_CLASS_NAME(Connection)
public:
  const std::shared_ptr<DataInput> in_;
  const std::shared_ptr<DataOutput> out;
  const std::shared_ptr<InputStream> sockIn;
  const std::shared_ptr<BufferedOutputStream> bos;
  const std::shared_ptr<Socket> s;
  const int destTCPPort;
  int64_t lastKeepAliveNS = System::nanoTime();

  Connection(int tcpPort) ;

  virtual void flush() ;

  virtual ~Connection();
};

} // #include  "core/src/java/org/apache/lucene/replicator/nrt/
