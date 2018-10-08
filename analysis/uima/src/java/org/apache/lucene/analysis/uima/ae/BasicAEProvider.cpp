using namespace std;

#include "BasicAEProvider.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"

namespace org::apache::lucene::analysis::uima::ae
{
using IOUtils = org::apache::lucene::util::IOUtils;
using org::apache::uima::UIMAFramework;
using org::apache::uima::analysis_engine::AnalysisEngine;
using org::apache::uima::analysis_engine::AnalysisEngineDescription;
using org::apache::uima::resource::ResourceInitializationException;
using org::apache::uima::util::XMLInputSource;

BasicAEProvider::BasicAEProvider(const wstring &aePath) : aePath(aePath) {}

shared_ptr<AnalysisEngine>
BasicAEProvider::getAE() 
{
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    if (cachedDescription == nullptr) {
      shared_ptr<XMLInputSource> in_ = nullptr;
      bool success = false;
      try {
        // get Resource Specifier from XML file
        in_ = getInputSource();

        // get AE description
        cachedDescription =
            UIMAFramework::getXMLParser().parseAnalysisEngineDescription(in_);
        configureDescription(cachedDescription);
        success = true;
      } catch (const runtime_error &e) {
        throw make_shared<ResourceInitializationException>(e);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        if (success) {
          try {
            IOUtils::close({in_->getInputStream()});
          } catch (const IOException &e) {
            throw make_shared<ResourceInitializationException>(e);
          }
        } else if (in_ != nullptr) {
          IOUtils::closeWhileHandlingException({in_->getInputStream()});
        }
      }
    }
  }

  return UIMAFramework::produceAnalysisEngine(cachedDescription);
}

void BasicAEProvider::configureDescription(
    shared_ptr<AnalysisEngineDescription> description)
{
  // no configuration
}

shared_ptr<XMLInputSource> BasicAEProvider::getInputSource() 
{
  try {
    return make_shared<XMLInputSource>(aePath);
  } catch (const runtime_error &e) {
    return make_shared<XMLInputSource>(getClass().getResource(aePath));
  }
}
} // namespace org::apache::lucene::analysis::uima::ae