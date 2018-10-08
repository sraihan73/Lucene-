using namespace std;

#include "StoredFieldVisitor.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using DocumentStoredFieldVisitor =
    org::apache::lucene::document::DocumentStoredFieldVisitor;

StoredFieldVisitor::StoredFieldVisitor() {}

void StoredFieldVisitor::binaryField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &value) 
{
}

void StoredFieldVisitor::stringField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &value) 
{
}

void StoredFieldVisitor::intField(shared_ptr<FieldInfo> fieldInfo,
                                  int value) 
{
}

void StoredFieldVisitor::longField(shared_ptr<FieldInfo> fieldInfo,
                                   int64_t value) 
{
}

void StoredFieldVisitor::floatField(shared_ptr<FieldInfo> fieldInfo,
                                    float value) 
{
}

void StoredFieldVisitor::doubleField(shared_ptr<FieldInfo> fieldInfo,
                                     double value) 
{
}
} // namespace org::apache::lucene::index