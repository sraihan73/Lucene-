using namespace std;

#include "ReplicatorTestCase.h"

namespace org::apache::lucene::replicator
{
using com::carrotsearch::randomizedtesting::annotations::ThreadLeakLingering;
using org::apache::http::conn::HttpClientConnectionManager;
using org::apache::http::impl::conn::PoolingHttpClientConnectionManager;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::eclipse::jetty::server::Connector;
using org::eclipse::jetty::server::Handler;
using org::eclipse::jetty::server::HttpConfiguration;
using org::eclipse::jetty::server::HttpConnectionFactory;
using org::eclipse::jetty::server::SecureRequestCustomizer;
using org::eclipse::jetty::server::Server;
using org::eclipse::jetty::server::ServerConnector;
using org::eclipse::jetty::server::SslConnectionFactory;
using org::eclipse::jetty::server::session::DefaultSessionIdManager;
using org::eclipse::jetty::util::ssl::SslContextFactory;
using org::eclipse::jetty::util::thread::QueuedThreadPool;
using org::junit::AfterClass;
shared_ptr<org::apache::http::conn::HttpClientConnectionManager>
    ReplicatorTestCase::clientConnectionManager;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClassReplicatorTestCase()
// throws Exception
void ReplicatorTestCase::afterClassReplicatorTestCase() 
{
  if (clientConnectionManager != nullptr) {
    clientConnectionManager->shutdown();
    clientConnectionManager.reset();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Server> ReplicatorTestCase::newHttpServer(
    shared_ptr<Handler> handler) 
{
  // if this property is true, then jetty will be configured to use SSL
  // leveraging the same system properties as java to specify
  // the keystore/truststore if they are set
  //
  // This means we will use the same truststore, keystore (and keys) for
  // the server as well as any client actions taken by this JVM in
  // talking to that server, but for the purposes of testing that should
  // be good enough
  constexpr bool useSsl = Boolean::getBoolean(L"tests.jettySsl");
  shared_ptr<SslContextFactory> *const sslcontext =
      make_shared<SslContextFactory>(false);

  if (useSsl) {
    if (nullptr != System::getProperty(L"javax.net.ssl.keyStore")) {
      sslcontext->setKeyStorePath(
          System::getProperty(L"javax.net.ssl.keyStore"));
    }
    if (nullptr != System::getProperty(L"javax.net.ssl.keyStorePassword")) {
      sslcontext->setKeyStorePassword(
          System::getProperty(L"javax.net.ssl.keyStorePassword"));
    }
    if (nullptr != System::getProperty(L"javax.net.ssl.trustStore")) {
      sslcontext->setKeyStorePath(
          System::getProperty(L"javax.net.ssl.trustStore"));
    }
    if (nullptr != System::getProperty(L"javax.net.ssl.trustStorePassword")) {
      sslcontext->setTrustStorePassword(
          System::getProperty(L"javax.net.ssl.trustStorePassword"));
    }
    sslcontext->setNeedClientAuth(
        Boolean::getBoolean(L"tests.jettySsl.clientAuth"));
  }

  shared_ptr<QueuedThreadPool> *const threadPool =
      make_shared<QueuedThreadPool>();
  threadPool->setDaemon(true);
  threadPool->setMaxThreads(10000);
  threadPool->setIdleTimeout(5000);
  threadPool->setStopTimeout(30000);

  shared_ptr<Server> server = make_shared<Server>(threadPool);
  server->setStopAtShutdown(true);
  server->manage(threadPool);

  shared_ptr<ServerConnector> *const connector;
  if (useSsl) {
    shared_ptr<HttpConfiguration> configuration =
        make_shared<HttpConfiguration>();
    configuration->setSecureScheme(L"https");
    configuration->addCustomizer(make_shared<SecureRequestCustomizer>());
    shared_ptr<ServerConnector> c = make_shared<ServerConnector>(
        server, make_shared<SslConnectionFactory>(sslcontext, L"http/1.1"),
        make_shared<HttpConnectionFactory>(configuration));
    connector = c;
  } else {
    shared_ptr<ServerConnector> c = make_shared<ServerConnector>(
        server, make_shared<HttpConnectionFactory>());
    connector = c;
  }

  connector->setPort(0);
  connector->setHost(L"127.0.0.1");

  server->setConnectors(std::deque<std::shared_ptr<Connector>>{connector});
  server->setSessionIdManager(make_shared<DefaultSessionIdManager>(
      server, make_shared<Random>(random()->nextLong())));
  server->setHandler(handler);

  server->start();

  return server;
}

int ReplicatorTestCase::serverPort(shared_ptr<Server> server)
{
  return (std::static_pointer_cast<ServerConnector>(server->getConnectors()[0]))
      ->getLocalPort();
}

wstring ReplicatorTestCase::serverHost(shared_ptr<Server> server)
{
  return (std::static_pointer_cast<ServerConnector>(server->getConnectors()[0]))
      ->getHost();
}

void ReplicatorTestCase::stopHttpServer(shared_ptr<Server> httpServer) throw(
    runtime_error)
{
  httpServer->stop();
  httpServer->join();
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<HttpClientConnectionManager>
ReplicatorTestCase::getClientConnectionManager()
{
  if (clientConnectionManager == nullptr) {
    shared_ptr<PoolingHttpClientConnectionManager> ccm =
        make_shared<PoolingHttpClientConnectionManager>();
    ccm->setDefaultMaxPerRoute(128);
    ccm->setMaxTotal(128);
    clientConnectionManager = ccm;
  }

  return clientConnectionManager;
}
} // namespace org::apache::lucene::replicator