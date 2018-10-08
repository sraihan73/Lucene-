using namespace std;

#include "HttpClientBase.h"

namespace org::apache::lucene::replicator::http
{
using org::apache::http::HttpEntity;
using org::apache::http::HttpResponse;
using org::apache::http::HttpStatus;
using org::apache::http::StatusLine;
using org::apache::http::client::config::RequestConfig;
using org::apache::http::client::methods::HttpGet;
using org::apache::http::client::methods::HttpPost;
using org::apache::http::conn::HttpClientConnectionManager;
using org::apache::http::impl::client::CloseableHttpClient;
using org::apache::http::impl::client::HttpClientBuilder;
using org::apache::http::util::EntityUtils;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using IOUtils = org::apache::lucene::util::IOUtils;

HttpClientBase::HttpClientBase(const wstring &host, int port,
                               const wstring &path,
                               shared_ptr<HttpClientConnectionManager> conMgr,
                               shared_ptr<RequestConfig> defaultConfig)
    : url(normalizedURL(host, port, path)),
      httpc(HttpClientBuilder::create()
                .setConnectionManager(conMgr)
                .setDefaultRequestConfig(this->defaultConfig)
                .build())
{
  if (defaultConfig == nullptr) {
    this->defaultConfig =
        RequestConfig::custom()
            .setConnectionRequestTimeout(DEFAULT_CONNECTION_TIMEOUT)
            .setSocketTimeout(DEFAULT_SO_TIMEOUT)
            .build();
  } else {
    this->defaultConfig = defaultConfig;
  }
}

void HttpClientBase::ensureOpen() 
{
  if (closed) {
    throw make_shared<AlreadyClosedException>(L"HttpClient already closed");
  }
}

wstring HttpClientBase::normalizedURL(const wstring &host, int port,
                                      const wstring &path)
{
  if (path == L"" || path.length() == 0) {
    path = L"/";
  }
  return L"http://" + host + L":" + to_wstring(port) + path;
}

void HttpClientBase::verifyStatus(shared_ptr<HttpResponse> response) throw(
    IOException)
{
  shared_ptr<StatusLine> statusLine = response->getStatusLine();
  if (statusLine->getStatusCode() != HttpStatus::SC_OK) {
    try {
      throwKnownError(response, statusLine);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      EntityUtils::consumeQuietly(response->getEntity());
    }
  }
}

void HttpClientBase::throwKnownError(
    shared_ptr<HttpResponse> response,
    shared_ptr<StatusLine> statusLine) 
{
  shared_ptr<ObjectInputStream> in_ = nullptr;
  try {
    in_ = make_shared<ObjectInputStream>(response->getEntity().getContent());
  } catch (const runtime_error &t) {
    // the response stream is not an exception - could be an error in
    // servlet.init().
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Unknown error: " + statusLine,
    // t);
    throw runtime_error(L"Unknown error: " + statusLine);
  }

  runtime_error t;
  try {
    t = static_cast<runtime_error>(in_->readObject());
    assert(t != nullptr);
  } catch (const runtime_error &th) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Failed to read exception
    // object: " + statusLine, th);
    throw runtime_error(L"Failed to read exception object: " + statusLine);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    in_->close();
  }
  throw IOUtils::rethrowAlways(t);
}

shared_ptr<HttpResponse>
HttpClientBase::executePOST(const wstring &request,
                            shared_ptr<HttpEntity> entity,
                            deque<wstring> &params) 
{
  ensureOpen();
  shared_ptr<HttpPost> m =
      make_shared<HttpPost>(queryString(request, {params}));
  m->setEntity(entity);
  shared_ptr<HttpResponse> response = httpc->execute(m);
  verifyStatus(response);
  return response;
}

shared_ptr<HttpResponse>
HttpClientBase::executeGET(const wstring &request,
                           deque<wstring> &params) 
{
  ensureOpen();
  shared_ptr<HttpGet> m = make_shared<HttpGet>(queryString(request, {params}));
  shared_ptr<HttpResponse> response = httpc->execute(m);
  verifyStatus(response);
  return response;
}

wstring HttpClientBase::queryString(
    const wstring &request,
    deque<wstring> &params) 
{
  shared_ptr<StringBuilder> query = (make_shared<StringBuilder>(url))
                                        ->append(L'/')
                                        ->append(request)
                                        ->append(L'?');
  if (params != nullptr) {
    for (int i = 0; i < params->length; i += 2) {
      query->append(params[i])
          ->append(L'=')
          ->append(URLEncoder::encode(params[i + 1], L"UTF8"))
          ->append(L'&');
    }
  }
  return query->substr(0, query->length() - 1);
}

shared_ptr<InputStream> HttpClientBase::responseInputStream(
    shared_ptr<HttpResponse> response) 
{
  return responseInputStream(response, false);
}

shared_ptr<InputStream>
HttpClientBase::responseInputStream(shared_ptr<HttpResponse> response,
                                    bool consume) 
{
  shared_ptr<HttpEntity> *const entity = response->getEntity();
  shared_ptr<InputStream> *const in_ = entity->getContent();
  if (!consume) {
    return in_;
  }
  return make_shared<InputStreamAnonymousInnerClass>(shared_from_this(), entity,
                                                     in_);
}

HttpClientBase::InputStreamAnonymousInnerClass::InputStreamAnonymousInnerClass(
    shared_ptr<HttpClientBase> outerInstance, shared_ptr<HttpEntity> entity,
    shared_ptr<InputStream> in_)
{
  this->outerInstance = outerInstance;
  this->entity = entity;
  this->in_ = in_;
  consumed = false;
}

int HttpClientBase::InputStreamAnonymousInnerClass::read() 
{
  constexpr int res = in_->read();
  consume(res);
  return res;
}

HttpClientBase::InputStreamAnonymousInnerClass::
    ~InputStreamAnonymousInnerClass()
{
  in_->close();
  consume(-1);
}

int HttpClientBase::InputStreamAnonymousInnerClass::read(
    std::deque<char> &b) 
{
  constexpr int res = in_->read(b);
  consume(res);
  return res;
}

int HttpClientBase::InputStreamAnonymousInnerClass::read(
    std::deque<char> &b, int off, int len) 
{
  constexpr int res = in_->read(b, off, len);
  consume(res);
  return res;
}

void HttpClientBase::InputStreamAnonymousInnerClass::consume(int minusOne)
{
  if (!consumed && minusOne == -1) {
    try {
      EntityUtils::consume(entity);
    } catch (const runtime_error &e) {
      // ignored on purpose
    }
    consumed = true;
  }
}

bool HttpClientBase::isClosed() { return closed; }

template <typename T>
T HttpClientBase::doAction(shared_ptr<HttpResponse> response,
                           shared_ptr<Callable<T>> call) 
{
  return doAction(response, true, call);
}

template <typename T>
T HttpClientBase::doAction(shared_ptr<HttpResponse> response, bool consume,
                           shared_ptr<Callable<T>> call) 
{
  runtime_error th = nullptr;
  try {
    return call->call();
  } catch (const runtime_error &t) {
    th = t;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    try {
      verifyStatus(response);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (consume) {
        EntityUtils::consumeQuietly(response->getEntity());
      }
    }
  }
  throw IOUtils::rethrowAlways(th);
}

HttpClientBase::~HttpClientBase()
{
  httpc->close();
  closed = true;
}
} // namespace org::apache::lucene::replicator::http