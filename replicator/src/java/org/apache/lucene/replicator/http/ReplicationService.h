#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/replicator/Replicator.h"

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
namespace org::apache::lucene::replicator::http
{

using Replicator = org::apache::lucene::replicator::Replicator;

/**
 * A server-side service for handling replication requests. The service assumes
 * requests are sent in the format
 * <code>/&lt;context&gt;/&lt;shard&gt;/&lt;action&gt;</code> where
 * <ul>
 * <li>{@code context} is the servlet context, e.g. {@link #REPLICATION_CONTEXT}
 * <li>{@code shard} is the ID of the shard, e.g. "s1"
 * <li>{@code action} is one of {@link ReplicationAction} values
 * </ul>
 * For example, to check whether there are revision updates for shard "s1" you
 * should send the request: <code>http://host:port/replicate/s1/update</code>.
 * <p>
 * This service is written like a servlet, and
 * {@link #perform(HttpServletRequest, HttpServletResponse)} takes servlet
 * request and response accordingly, so it is quite easy to embed in your
 * application's servlet.
 *
 * @lucene.experimental
 */
class ReplicationService
    : public std::enable_shared_from_this<ReplicationService>
{
  GET_CLASS_NAME(ReplicationService)

  /** Actions supported by the {@link ReplicationService}. */
public:
  enum class ReplicationAction {
    GET_CLASS_NAME(ReplicationAction) OBTAIN,
    RELEASE,
    UPDATE
  };

  /** The context path for the servlet. */
public:
  static const std::wstring REPLICATION_CONTEXT;

  /** Request parameter name for providing the revision version. */
  static const std::wstring REPLICATE_VERSION_PARAM;

  /** Request parameter name for providing a session ID. */
  static const std::wstring REPLICATE_SESSION_ID_PARAM;

  /** Request parameter name for providing the file's source. */
  static const std::wstring REPLICATE_SOURCE_PARAM;

  /** Request parameter name for providing the file's name. */
  static const std::wstring REPLICATE_FILENAME_PARAM;

private:
  static constexpr int SHARD_IDX = 0, ACTION_IDX = 1;

  const std::unordered_map<std::wstring, std::shared_ptr<Replicator>>
      replicators;

public:
  // C++ TODO: No base class can be determined:
  ReplicationService(
      std::unordered_map<std::wstring, std::shared_ptr<Replicator>>
          &replicators); // super();

  /**
   * Returns the path elements that were given in the servlet request, excluding
   * the servlet's action context.
   */
private:
  std::deque<std::wstring>
  getPathElements(std::shared_ptr<HttpServletRequest> req);

  static std::wstring
  extractRequestParam(std::shared_ptr<HttpServletRequest> req,
                      const std::wstring &paramName) ;

  static void copy(std::shared_ptr<InputStream> in_,
                   std::shared_ptr<OutputStream> out) ;

  /** Executes the replication task. */
public:
  virtual void
  perform(std::shared_ptr<HttpServletRequest> req,
          std::shared_ptr<HttpServletResponse> resp) throw(ServletException,
                                                           IOException);
};

} // #include  "core/src/java/org/apache/lucene/replicator/http/
