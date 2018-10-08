using namespace std;

#include "LuceneDictionary.h"

namespace org::apache::lucene::search::spell
{
using IndexReader = org::apache::lucene::index::IndexReader;
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using Terms = org::apache::lucene::index::Terms;
using MultiFields = org::apache::lucene::index::MultiFields;

LuceneDictionary::LuceneDictionary(shared_ptr<IndexReader> reader,
                                   const wstring &field)
{
  this->reader = reader;
  this->field = field;
}

shared_ptr<InputIterator>
LuceneDictionary::getEntryIterator() 
{
  shared_ptr<Terms> *const terms = MultiFields::getTerms(reader, field);
  if (terms != nullptr) {
    return make_shared<InputIterator::InputIteratorWrapper>(terms->begin());
  } else {
    return InputIterator::EMPTY;
  }
}
} // namespace org::apache::lucene::search::spell