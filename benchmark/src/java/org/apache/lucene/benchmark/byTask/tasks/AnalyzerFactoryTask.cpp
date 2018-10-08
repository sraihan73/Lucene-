using namespace std;

#include "AnalyzerFactoryTask.h"
#include "../../../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/util/AbstractAnalysisFactory.h"
#include "../../../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/util/CharFilterFactory.h"
#include "../../../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/util/FilesystemResourceLoader.h"
#include "../../../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/util/ResourceLoaderAware.h"
#include "../../../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "../../../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../PerfRunData.h"
#include "../utils/AnalyzerFactory.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using FilesystemResourceLoader =
    org::apache::lucene::analysis::util::FilesystemResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using AnalyzerFactory =
    org::apache::lucene::benchmark::byTask::utils::AnalyzerFactory;
using Version = org::apache::lucene::util::Version;
const wstring AnalyzerFactoryTask::LUCENE_ANALYSIS_PACKAGE_PREFIX =
    L"org.apache.lucene.analysis.";
const shared_ptr<java::util::regex::Pattern>
    AnalyzerFactoryTask::ANALYSIS_COMPONENT_SUFFIX_PATTERN =
        java::util::regex::Pattern::compile(
            L"(?s:(?:(?:Token|Char)?Filter|Tokenizer)(?:Factory)?)$");
const shared_ptr<java::util::regex::Pattern>
    AnalyzerFactoryTask::TRAILING_DOT_ZERO_PATTERN =
        java::util::regex::Pattern::compile(L"\\.0$");

AnalyzerFactoryTask::AnalyzerFactoryTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

int AnalyzerFactoryTask::doLogic() { return 1; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @SuppressWarnings("fallthrough") public void
// setParams(std::wstring params)
void AnalyzerFactoryTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);
  ArgType expectedArgType = ArgType::ANALYZER_ARG;

  shared_ptr<StreamTokenizer> *const stok =
      make_shared<StreamTokenizer>(make_shared<StringReader>(params));
  stok->commentChar(L'#');
  stok->quoteChar(L'"');
  stok->quoteChar(L'\'');
  stok->eolIsSignificant(false);
  stok->ordinaryChar(L'(');
  stok->ordinaryChar(L')');
  stok->ordinaryChar(L':');
  stok->ordinaryChar(L',');
  try {
    while (stok->nextToken() != StreamTokenizer::TT_EOF) {
      switch (stok->ttype) {
      case L',': {
        // Do nothing
        break;
      }
      case StreamTokenizer::TT_WORD: {
        if (expectedArgType.equals(ArgType::ANALYZER_ARG)) {
          const wstring argName = stok->sval;
          // C++ TODO: The following Java case-insensitive std::wstring method call is
          // not converted:
          if (!argName.equalsIgnoreCase(L"name") &&
              !argName.equalsIgnoreCase(L"positionIncrementGap") &&
              !argName.equalsIgnoreCase(L"offsetGap")) {
            throw runtime_error(
                L"Line #" + to_wstring(lineno(stok)) +
                L": Missing 'name' param to AnalyzerFactory: '" + params +
                L"'");
          }
          stok->nextToken();
          if (stok->ttype != L':') {
            throw runtime_error(L"Line #" + to_wstring(lineno(stok)) +
                                L": Missing ':' after '" + argName +
                                L"' param to AnalyzerFactory");
          }

          stok->nextToken();
          wstring argValue = stok->sval;
          switch (stok->ttype) {
          case StreamTokenizer::TT_NUMBER: {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            argValue = Double::toString(stok->nval);
            // Drop the ".0" from numbers, for integer arguments
            argValue =
                TRAILING_DOT_ZERO_PATTERN->matcher(argValue).replaceFirst(L"");
            // Intentional fallthrough
          }
          case L'"':
          case L'\'':
          case StreamTokenizer::TT_WORD: {
            // C++ TODO: The following Java case-insensitive std::wstring method call
            // is not converted:
            if (argName.equalsIgnoreCase(L"name")) {
              factoryName = argValue;
              expectedArgType =
                  ArgType::ANALYZER_ARG_OR_CHARFILTER_OR_TOKENIZER;
            } else {
              int intArgValue = 0;
              try {
                intArgValue = stoi(argValue);
              } catch (const NumberFormatException &e) {
                // C++ TODO: This exception's constructor requires only one
                // argument: ORIGINAL LINE: throw new RuntimeException("Line #" +
                // lineno(stok) + ": Exception parsing " + argName + " value '" +
                // argValue + "'", e);
                throw runtime_error(L"Line #" + to_wstring(lineno(stok)) +
                                    L": Exception parsing " + argName +
                                    L" value '" + argValue + L"'");
              }
              // C++ TODO: The following Java case-insensitive std::wstring method
              // call is not converted:
              if (argName.equalsIgnoreCase(L"positionIncrementGap")) {
                positionIncrementGap = intArgValue;
              }
              // C++ TODO: The following Java case-insensitive std::wstring method
              // call is not converted:
              else if (argName.equalsIgnoreCase(L"offsetGap")) {
                offsetGap = intArgValue;
              }
            }
            break;
          }
          case StreamTokenizer::TT_EOF: {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            throw runtime_error(L"Unexpected EOF: " + stok->toString());
          }
          default: {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            throw runtime_error(L"Line #" + to_wstring(lineno(stok)) +
                                L": Unexpected token: " + stok->toString());
          }
          }
        } else if (expectedArgType.equals(
                       ArgType::ANALYZER_ARG_OR_CHARFILTER_OR_TOKENIZER)) {
          const wstring argName = stok->sval;

          // C++ TODO: The following Java case-insensitive std::wstring method call is
          // not converted:
          if (argName.equalsIgnoreCase(L"positionIncrementGap") ||
              argName.equalsIgnoreCase(L"offsetGap")) {
            stok->nextToken();
            if (stok->ttype != L':') {
              throw runtime_error(L"Line #" + to_wstring(lineno(stok)) +
                                  L": Missing ':' after '" + argName +
                                  L"' param to AnalyzerFactory");
            }
            stok->nextToken();
            int intArgValue = static_cast<int>(stok->nval);
            switch (stok->ttype) {
            case L'"':
            case L'\'':
            case StreamTokenizer::TT_WORD: {
              intArgValue = 0;
              try {
                intArgValue = stoi(stok->sval->trim());
              } catch (const NumberFormatException &e) {
                // C++ TODO: This exception's constructor requires only one
                // argument: ORIGINAL LINE: throw new RuntimeException("Line #" +
                // lineno(stok) + ": Exception parsing " + argName + " value '" +
                // stok.sval + "'", e);
                throw runtime_error(L"Line #" + to_wstring(lineno(stok)) +
                                    L": Exception parsing " + argName +
                                    L" value '" + stok->sval + L"'");
              }
              // Intentional fall-through
            }
            case StreamTokenizer::TT_NUMBER: {
              // C++ TODO: The following Java case-insensitive std::wstring method
              // call is not converted:
              if (argName.equalsIgnoreCase(L"positionIncrementGap")) {
                positionIncrementGap = intArgValue;
              }
              // C++ TODO: The following Java case-insensitive std::wstring method
              // call is not converted:
              else if (argName.equalsIgnoreCase(L"offsetGap")) {
                offsetGap = intArgValue;
              }
              break;
            }
            case StreamTokenizer::TT_EOF: {
              // C++ TODO: There is no native C++ equivalent to 'toString':
              throw runtime_error(L"Unexpected EOF: " + stok->toString());
            }
            default: {
              // C++ TODO: There is no native C++ equivalent to 'toString':
              throw runtime_error(L"Line #" + to_wstring(lineno(stok)) +
                                  L": Unexpected token: " + stok->toString());
            }
            }
            break;
          }
          try {
            constexpr type_info clazz;
            clazz = lookupAnalysisClass(argName, CharFilterFactory::typeid);
            createAnalysisPipelineComponent(stok, clazz);
          } catch (const invalid_argument &e) {
            try {
              constexpr type_info clazz;
              clazz = lookupAnalysisClass(argName, TokenizerFactory::typeid);
              createAnalysisPipelineComponent(stok, clazz);
              expectedArgType = ArgType::TOKENFILTER;
            } catch (const invalid_argument &e2) {
              throw runtime_error(
                  L"Line #" + to_wstring(lineno(stok)) +
                  L": Can't find class '" + argName +
                  L"' as CharFilterFactory or TokenizerFactory");
            }
          }
        } else { // expectedArgType = ArgType.TOKENFILTER
          const wstring className = stok->sval;
          constexpr type_info clazz;
          try {
            clazz = lookupAnalysisClass(className, TokenFilterFactory::typeid);
          } catch (const invalid_argument &e) {
            throw runtime_error(L"Line #" + to_wstring(lineno(stok)) +
                                L": Can't find class '" + className +
                                L"' as TokenFilterFactory");
          }
          createAnalysisPipelineComponent(stok, clazz);
        }
        break;
      }
      default: {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        throw runtime_error(L"Line #" + to_wstring(lineno(stok)) +
                            L": Unexpected token: " + stok->toString());
      }
      }
    }
  } catch (const runtime_error &e) {
    if (e.what()->startsWith(L"Line #")) {
      throw e;
    } else {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("Line #" + lineno(stok) + ":
      // ", e);
      throw runtime_error(L"Line #" + to_wstring(lineno(stok)) + L": ");
    }
  } catch (const runtime_error &t) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Line #" + lineno(stok) + ": ",
    // t);
    throw runtime_error(L"Line #" + to_wstring(lineno(stok)) + L": ");
  }

  shared_ptr<AnalyzerFactory> *const analyzerFactory =
      make_shared<AnalyzerFactory>(charFilterFactories, tokenizerFactory,
                                   tokenFilterFactories);
  analyzerFactory->setPositionIncrementGap(positionIncrementGap);
  analyzerFactory->setOffsetGap(offsetGap);
  getRunData()->getAnalyzerFactories().emplace(factoryName, analyzerFactory);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") private void
// createAnalysisPipelineComponent(java.io.StreamTokenizer stok, Class clazz)
void AnalyzerFactoryTask::createAnalysisPipelineComponent(
    shared_ptr<StreamTokenizer> stok, type_info clazz)
{
  unordered_map<wstring, wstring> argMap = unordered_map<wstring, wstring>();
  bool parenthetical = false;
  try {
    while (stok->nextToken() != StreamTokenizer::TT_EOF) {
      switch (stok->ttype) {
      case L',': {
        if (parenthetical) {
          // Do nothing
          break;
        } else {
          // Finished reading this analysis factory configuration
          goto WHILE_LOOPBreak;
        }
      }
      case L'(': {
        if (parenthetical) {
          throw runtime_error(L"Line #" + to_wstring(lineno(stok)) +
                              L": Unexpected opening parenthesis.");
        }
        parenthetical = true;
        break;
      }
      case L')': {
        if (parenthetical) {
          parenthetical = false;
        } else {
          throw runtime_error(L"Line #" + to_wstring(lineno(stok)) +
                              L": Unexpected closing parenthesis.");
        }
        break;
      }
      case StreamTokenizer::TT_WORD: {
        if (!parenthetical) {
          throw runtime_error(L"Line #" + to_wstring(lineno(stok)) +
                              L": Unexpected token '" + stok->sval + L"'");
        }
        wstring argName = stok->sval;
        stok->nextToken();
        if (stok->ttype != L':') {
          throw runtime_error(L"Line #" + to_wstring(lineno(stok)) +
                              L": Missing ':' after '" + argName +
                              L"' param to " + clazz.name());
        }
        stok->nextToken();
        wstring argValue = stok->sval;
        switch (stok->ttype) {
        case StreamTokenizer::TT_NUMBER: {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          argValue = Double::toString(stok->nval);
          // Drop the ".0" from numbers, for integer arguments
          argValue =
              TRAILING_DOT_ZERO_PATTERN->matcher(argValue).replaceFirst(L"");
          // Intentional fall-through
        }
        case L'"':
        case L'\'':
        case StreamTokenizer::TT_WORD: {
          argMap.emplace(argName, argValue);
          break;
        }
        case StreamTokenizer::TT_EOF: {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          throw runtime_error(L"Unexpected EOF: " + stok->toString());
        }
        default: {
          // C++ TODO: There is no native C++ equivalent to 'toString':
          throw runtime_error(L"Line #" + to_wstring(lineno(stok)) +
                              L": Unexpected token: " + stok->toString());
        }
        }
      }
      }
    WHILE_LOOPContinue:;
    }
  WHILE_LOOPBreak:
    if (argMap.find(L"luceneMatchVersion") == argMap.end()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      argMap.emplace(L"luceneMatchVersion", Version::LATEST->toString());
    }
    shared_ptr<AbstractAnalysisFactory> *const instance;
    try {
      instance =
          clazz.getConstructor(unordered_map::typeid).newInstance(argMap);
    } catch (const runtime_error &e) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("Line #" + lineno(stok) + ":
      // ", e);
      throw runtime_error(L"Line #" + to_wstring(lineno(stok)) + L": ");
    }
    if (std::dynamic_pointer_cast<ResourceLoaderAware>(instance) != nullptr) {
      shared_ptr<Path> baseDir =
          Paths->get(getRunData()->getConfig()->get(L"work.dir", L"work"));
      if (!Files::isDirectory(baseDir)) {
        baseDir = Paths->get(L".");
      }
      (std::static_pointer_cast<ResourceLoaderAware>(instance))
          ->inform(make_shared<FilesystemResourceLoader>(baseDir));
    }
    if (CharFilterFactory::typeid->isAssignableFrom(clazz)) {
      charFilterFactories.push_back(
          std::static_pointer_cast<CharFilterFactory>(instance));
    } else if (TokenizerFactory::typeid->isAssignableFrom(clazz)) {
      tokenizerFactory = std::static_pointer_cast<TokenizerFactory>(instance);
    } else if (TokenFilterFactory::typeid->isAssignableFrom(clazz)) {
      tokenFilterFactories.push_back(
          std::static_pointer_cast<TokenFilterFactory>(instance));
    }
  } catch (const runtime_error &e) {
    if (e.what()->startsWith(L"Line #")) {
      ;
    } else {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("Line #" + lineno(stok) + ":
      // ", e);
      throw runtime_error(L"Line #" + to_wstring(lineno(stok)) + L": ");
    }
  } catch (const runtime_error &t) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Line #" + lineno(stok) + ": ",
    // t);
    throw runtime_error(L"Line #" + to_wstring(lineno(stok)) + L": ");
  }
}

template <typename T>
type_info AnalyzerFactoryTask::lookupAnalysisClass(
    const wstring &className,
    type_info<T> &expectedType) 
{
  if (className.find(L".") != wstring::npos) {
    try {
      // First, try className == FQN
      return type_info::forName(className).asSubclass(expectedType);
    } catch (const ClassNotFoundException &e) {
      try {
        // Second, retry lookup after prepending the Lucene analysis package
        // prefix
        return type_info::forName(LUCENE_ANALYSIS_PACKAGE_PREFIX + className)
            .asSubclass(expectedType);
      } catch (const ClassNotFoundException &e1) {
        throw make_shared<ClassNotFoundException>(
            L"Can't find class '" + className + L"' or '" +
            LUCENE_ANALYSIS_PACKAGE_PREFIX + className + L"'");
      }
    }
  }
  // No dot - use analysis SPI lookup
  const wstring analysisComponentName =
      ANALYSIS_COMPONENT_SUFFIX_PATTERN->matcher(className).replaceFirst(L"");
  if (CharFilterFactory::typeid->isAssignableFrom(expectedType)) {
    return CharFilterFactory::lookupClass(analysisComponentName)
        .asSubclass(expectedType);
  } else if (TokenizerFactory::typeid->isAssignableFrom(expectedType)) {
    return TokenizerFactory::lookupClass(analysisComponentName)
        .asSubclass(expectedType);
  } else if (TokenFilterFactory::typeid->isAssignableFrom(expectedType)) {
    return TokenFilterFactory::lookupClass(analysisComponentName)
        .asSubclass(expectedType);
  }

  throw make_shared<ClassNotFoundException>(L"Can't find class '" + className +
                                            L"'");
}

bool AnalyzerFactoryTask::supportsParams() { return true; }

int AnalyzerFactoryTask::lineno(shared_ptr<StreamTokenizer> stok)
{
  return getAlgLineNum() + stok->lineno();
}
} // namespace org::apache::lucene::benchmark::byTask::tasks