using namespace std;

#include "CompletionTerms.h"

namespace org::apache::lucene::search::suggest::document
{
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using Terms = org::apache::lucene::index::Terms;

CompletionTerms::CompletionTerms(shared_ptr<Terms> in_,
                                 shared_ptr<CompletionsTermsReader> reader)
    : org::apache::lucene::index::FilterLeafReader::FilterTerms(in_),
      reader(reader)
{
}

char CompletionTerms::getType()
{
  return (reader != nullptr) ? reader->type : SuggestField::TYPE;
}

int64_t CompletionTerms::getMinWeight()
{
  return (reader != nullptr) ? reader->minWeight : 0;
}

int64_t CompletionTerms::getMaxWeight()
{
  return (reader != nullptr) ? reader->maxWeight : 0;
}

shared_ptr<NRTSuggester> CompletionTerms::suggester() 
{
  return (reader != nullptr) ? reader->suggester() : nullptr;
}
} // namespace org::apache::lucene::search::suggest::document