using namespace std;

#include "ReplicationService.h"

namespace org::apache::lucene::replicator::http
{
using org::apache::http::HttpStatus;
using Replicator = org::apache::lucene::replicator::Replicator;
using SessionToken = org::apache::lucene::replicator::SessionToken;
const wstring ReplicationService::REPLICATION_CONTEXT = L"/replicate";
const wstring ReplicationService::REPLICATE_VERSION_PARAM = L"version";
const wstring ReplicationService::REPLICATE_SESSION_ID_PARAM = L"sessionid";
const wstring ReplicationService::REPLICATE_SOURCE_PARAM = L"source";
const wstring ReplicationService::REPLICATE_FILENAME_PARAM = L"filename";

// C++ TODO: No base class can be determined:
ReplicationService::ReplicationService(
    unordered_map<wstring, std::shared_ptr<Replicator>> &replicators)
    : replicators(replicators)
{
}

std::deque<wstring>
ReplicationService::getPathElements(shared_ptr<HttpServletRequest> req)
{
  wstring path = req->getServletPath();
  wstring pathInfo = req->getPathInfo();
  if (pathInfo != L"") {
    path += pathInfo;
  }
  int actionLen = REPLICATION_CONTEXT.length();
  int startIdx = actionLen;
  if (path.length() > actionLen && path[actionLen] == L'/') {
    ++startIdx;
  }

  // split the string on '/' and remove any empty elements. This is better
  // than using std::wstring.split() since the latter may return empty elements in
  // the array
  shared_ptr<StringTokenizer> stok =
      make_shared<StringTokenizer>(path.substr(startIdx), L"/");
  deque<wstring> elements = deque<wstring>();
  while (stok->hasMoreTokens()) {
    elements.push_back(stok->nextToken());
  }
  return elements.toArray(std::deque<wstring>(0));
}

wstring ReplicationService::extractRequestParam(
    shared_ptr<HttpServletRequest> req,
    const wstring &paramName) 
{
  wstring param = req->getParameter(paramName);
  if (param == L"") {
    // C++ TODO: The following line could not be converted:
    throw javax.servlet.ServletException(L"Missing mandatory parameter: " +
                                         paramName);
  }
  return param;
}

void ReplicationService::copy(shared_ptr<InputStream> in_,
                              shared_ptr<OutputStream> out) 
{
  std::deque<char> buf(16384);
  int numRead;
  while ((numRead = in_->read(buf)) != -1) {
    out->write(buf, 0, numRead);
  }
}

void ReplicationService::perform(
    shared_ptr<HttpServletRequest> req,
    shared_ptr<HttpServletResponse> resp) 
{
  std::deque<wstring> pathElements = getPathElements(req);

  if (pathElements.size() != 2) {
    // C++ TODO: The following line could not be converted:
    throw javax.servlet.ServletException(
        L"invalid path, must contain shard ID and action, e.g. */s1/update");
  }

  constexpr ReplicationAction action;
  try {
    action = ReplicationAction::valueOf(
        pathElements[ACTION_IDX].toUpperCase(Locale::ENGLISH));
  } catch (const invalid_argument &e) {
    // C++ TODO: The following line could not be converted:
    throw javax.servlet.ServletException(L"Unsupported action provided: " +
                                         pathElements[ACTION_IDX]);
  }

  shared_ptr<Replicator> *const replicator =
      replicators[pathElements[SHARD_IDX]];
  if (replicator == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw javax.servlet.ServletException(L"unrecognized shard ID " +
                                         pathElements[SHARD_IDX]);
  }

  // SOLR-8933 Don't close this stream.
  shared_ptr<ServletOutputStream> resOut = resp->getOutputStream();
  try {
    switch (action) {
    case org::apache::lucene::replicator::http::ReplicationService::
        ReplicationAction::OBTAIN: {
      const wstring sessionID =
          extractRequestParam(req, REPLICATE_SESSION_ID_PARAM);
      const wstring fileName =
          extractRequestParam(req, REPLICATE_FILENAME_PARAM);
      const wstring source = extractRequestParam(req, REPLICATE_SOURCE_PARAM);
      shared_ptr<InputStream> in_ =
          replicator->obtainFile(sessionID, source, fileName);
      try {
        copy(in_, resOut);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        in_->close();
      }
      break;
    }
    case org::apache::lucene::replicator::http::ReplicationService::
        ReplicationAction::RELEASE:
      replicator->release(extractRequestParam(req, REPLICATE_SESSION_ID_PARAM));
      break;
    case org::apache::lucene::replicator::http::ReplicationService::
        ReplicationAction::UPDATE: {
      wstring currVersion = req->getParameter(REPLICATE_VERSION_PARAM);
      shared_ptr<SessionToken> token = replicator->checkForUpdate(currVersion);
      if (token == nullptr) {
        resOut->write(0); // marker for null token
      } else {
        resOut->write(1);
        token->serialize(make_shared<DataOutputStream>(resOut));
      }
      break;
    }
    }
  } catch (const runtime_error &e) {
    resp->setStatus(
        HttpStatus::SC_INTERNAL_SERVER_ERROR); // propagate the failure
    try {
      /*
       * Note: it is assumed that "identified exceptions" are thrown before
       * anything was written to the stream.
       */
      shared_ptr<ObjectOutputStream> oos =
          make_shared<ObjectOutputStream>(resOut);
      oos->writeObject(e);
      oos->flush();
    } catch (const runtime_error &e2) {
      throw make_shared<IOException>(L"Could not serialize", e2);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    resp->flushBuffer();
  }
}
} // namespace org::apache::lucene::replicator::http