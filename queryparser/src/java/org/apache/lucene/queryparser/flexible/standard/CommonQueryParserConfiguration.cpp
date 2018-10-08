using namespace std;

#include "CommonQueryParserConfiguration.h"

namespace org::apache::lucene::queryparser::flexible::standard
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using DateTools = org::apache::lucene::document::DateTools;
using Resolution = org::apache::lucene::document::DateTools::Resolution;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
} // namespace org::apache::lucene::queryparser::flexible::standard