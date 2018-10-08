using namespace std;

#include "ReplicationServlet.h"

namespace org::apache::lucene::replicator::http
{

ReplicationServlet::ReplicationServlet(shared_ptr<ReplicationService> service)
    : service(service)
{
}

void ReplicationServlet::doGet(
    shared_ptr<HttpServletRequest> req,
    shared_ptr<HttpServletResponse> resp) 
{
  if (respondWithError) {
    resp->sendError(500, L"Fake error");
  } else {
    service->perform(req, resp);
  }
}

void ReplicationServlet::setRespondWithError(bool respondWithError)
{
  this->respondWithError = respondWithError;
}
} // namespace org::apache::lucene::replicator::http