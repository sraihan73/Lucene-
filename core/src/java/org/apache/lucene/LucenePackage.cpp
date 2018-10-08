using namespace std;

#include "LucenePackage.h"

namespace org::apache::lucene
{

LucenePackage::LucenePackage() {} // can't construct

shared_ptr<Package> LucenePackage::get()
{
  return LucenePackage::typeid->getPackage();
}
} // namespace org::apache::lucene