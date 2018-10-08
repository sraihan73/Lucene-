using namespace std;

#include "NewAnalyzerTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../PerfRunData.h"
#include "../utils/AnalyzerFactory.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using AnalyzerFactory =
    org::apache::lucene::benchmark::byTask::utils::AnalyzerFactory;
using Version = org::apache::lucene::util::Version;

NewAnalyzerTask::NewAnalyzerTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
  analyzerNames = deque<>();
}

shared_ptr<Analyzer>
NewAnalyzerTask::createAnalyzer(const wstring &className) 
{
  constexpr type_info clazz =
      type_info::forName(className).asSubclass(Analyzer::typeid);
  try {
    // first try to use a ctor with version parameter (needed for many new
    // Analyzers that have no default one anymore
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: Constructor<? extends org.apache.lucene.analysis.Analyzer>
    // cnstr = clazz.getConstructor(org.apache.lucene.util.Version.class);
    shared_ptr < Constructor < ? extends Analyzer >> cnstr =
                                     clazz.getConstructor(Version::typeid);
    return cnstr->newInstance(Version::LATEST);
  } catch (const NoSuchMethodException &nsme) {
    // otherwise use default ctor
    return clazz.newInstance();
  }
}

int NewAnalyzerTask::doLogic() 
{
  wstring analyzerName = L"";
  try {
    if (current >= analyzerNames.size()) {
      current = 0;
    }
    analyzerName = analyzerNames[current++];
    shared_ptr<Analyzer> analyzer = nullptr;
    if (L"" == analyzerName || 0 == analyzerName.length()) {
      analyzerName = L"org.apache.lucene.analysis.standard.StandardAnalyzer";
    }
    // First, lookup analyzerName as a named analyzer factory
    shared_ptr<AnalyzerFactory> factory =
        getRunData()->getAnalyzerFactories()[analyzerName];
    if (nullptr != factory) {
      analyzer = factory->create();
    } else {
      if (analyzerName.find(L".") != wstring::npos) {
        if (StringHelper::startsWith(analyzerName, L"standard.")) {
          analyzerName = L"org.apache.lucene.analysis." + analyzerName;
        }
        analyzer = createAnalyzer(analyzerName);
      } else { // No package
        try {
          // Attempt to instantiate a core analyzer
          wstring coreClassName =
              L"org.apache.lucene.analysis.core." + analyzerName;
          analyzer = createAnalyzer(coreClassName);
          analyzerName = coreClassName;
        } catch (const ClassNotFoundException &e) {
          // If not a core analyzer, try the base analysis package
          analyzerName = L"org.apache.lucene.analysis." + analyzerName;
          analyzer = createAnalyzer(analyzerName);
        }
      }
    }
    getRunData()->setAnalyzer(analyzer);
  } catch (const runtime_error &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Error creating Analyzer: " +
    // analyzerName, e);
    throw runtime_error(L"Error creating Analyzer: " + analyzerName);
  }
  return 1;
}

void NewAnalyzerTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  shared_ptr<StreamTokenizer> *const stok =
      make_shared<StreamTokenizer>(make_shared<StringReader>(params));
  stok->quoteChar(L'"');
  stok->quoteChar(L'\'');
  stok->eolIsSignificant(false);
  stok->ordinaryChar(L',');
  try {
    while (stok->nextToken() != StreamTokenizer::TT_EOF) {
      switch (stok->ttype) {
      case L',': {
        // Do nothing
        break;
      }
      case L'\'':
      case L'\"':
      case StreamTokenizer::TT_WORD: {
        analyzerNames.push_back(stok->sval);
        break;
      }
      default: {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        throw runtime_error(L"Unexpected token: " + stok->toString());
      }
      }
    }
  } catch (const runtime_error &e) {
    if (e.what()->startsWith(L"Line #")) {
      throw e;
    } else {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("Line #" + (stok.lineno() +
      // getAlgLineNum()) + ": ", e);
      throw runtime_error(L"Line #" + (stok->lineno() + getAlgLineNum()) +
                          L": ");
    }
  } catch (const runtime_error &t) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Line #" + (stok.lineno() +
    // getAlgLineNum()) + ": ", t);
    throw runtime_error(L"Line #" + (stok->lineno() + getAlgLineNum()) + L": ");
  }
}

bool NewAnalyzerTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks