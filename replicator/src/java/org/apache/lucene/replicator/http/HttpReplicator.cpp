using namespace std;

#include "HttpReplicator.h"

namespace org::apache::lucene::replicator::http
{
using org::apache::http::HttpResponse;
using org::apache::http::conn::HttpClientConnectionManager;
using Replicator = org::apache::lucene::replicator::Replicator;
using Revision = org::apache::lucene::replicator::Revision;
using SessionToken = org::apache::lucene::replicator::SessionToken;
using ReplicationAction = org::apache::lucene::replicator::http::
    ReplicationService::ReplicationAction;

HttpReplicator::HttpReplicator(const wstring &host, int port,
                               const wstring &path,
                               shared_ptr<HttpClientConnectionManager> conMgr)
    : HttpClientBase(host, port, path, conMgr, nullptr)
{
}

shared_ptr<SessionToken>
HttpReplicator::checkForUpdate(const wstring &currVersion) 
{
  std::deque<wstring> params;
  if (currVersion != L"") {
    params = std::deque<wstring>{ReplicationService::REPLICATE_VERSION_PARAM,
                                  currVersion};
  }
  shared_ptr<HttpResponse> *const response =
      executeGET(ReplicationAction::UPDATE.name(), params);
  return doAction(response, make_shared<CallableAnonymousInnerClass>(
                                shared_from_this(), response));
}

HttpReplicator::CallableAnonymousInnerClass::CallableAnonymousInnerClass(
    shared_ptr<HttpReplicator> outerInstance, shared_ptr<HttpResponse> response)
{
  this->outerInstance = outerInstance;
  this->response = response;
}

shared_ptr<SessionToken>
HttpReplicator::CallableAnonymousInnerClass::call() 
{
  shared_ptr<DataInputStream> *const dis = make_shared<DataInputStream>(
      outerInstance->responseInputStream(response));
  try {
    if (dis->readByte() == 0) {
      return nullptr;
    } else {
      return make_shared<SessionToken>(dis);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    dis->close();
  }
}

shared_ptr<InputStream>
HttpReplicator::obtainFile(const wstring &sessionID, const wstring &source,
                           const wstring &fileName) 
{
  std::deque<wstring> params = {
      ReplicationService::REPLICATE_SESSION_ID_PARAM, sessionID,
      ReplicationService::REPLICATE_SOURCE_PARAM,     source,
      ReplicationService::REPLICATE_FILENAME_PARAM,   fileName};
  shared_ptr<HttpResponse> *const response =
      executeGET(ReplicationAction::OBTAIN.name(), params);
  return doAction(
      response, false,
      make_shared<CallableAnonymousInnerClass2>(shared_from_this(), response));
}

HttpReplicator::CallableAnonymousInnerClass2::CallableAnonymousInnerClass2(
    shared_ptr<HttpReplicator> outerInstance, shared_ptr<HttpResponse> response)
{
  this->outerInstance = outerInstance;
  this->response = response;
}

shared_ptr<InputStream>
HttpReplicator::CallableAnonymousInnerClass2::call() 
{
  return outerInstance->responseInputStream(response, true);
}

void HttpReplicator::publish(shared_ptr<Revision> revision) 
{
  throw make_shared<UnsupportedOperationException>(
      L"this replicator implementation does not support remote publishing of "
      L"revisions");
}

void HttpReplicator::release(const wstring &sessionID) 
{
  std::deque<wstring> params = {ReplicationService::REPLICATE_SESSION_ID_PARAM,
                                 sessionID};
  shared_ptr<HttpResponse> *const response =
      executeGET(ReplicationAction::RELEASE.name(), params);
  doAction(response,
           make_shared<CallableAnonymousInnerClass3>(shared_from_this()));
}

HttpReplicator::CallableAnonymousInnerClass3::CallableAnonymousInnerClass3(
    shared_ptr<HttpReplicator> outerInstance)
{
  this->outerInstance = outerInstance;
}

any HttpReplicator::CallableAnonymousInnerClass3::call() 
{
  return nullptr; // do not remove this call: as it is still validating for us!
}
} // namespace org::apache::lucene::replicator::http