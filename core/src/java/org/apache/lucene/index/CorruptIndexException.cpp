using namespace std;

#include "CorruptIndexException.h"

namespace org::apache::lucene::index
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;

CorruptIndexException::CorruptIndexException(const wstring &message,
                                             shared_ptr<DataInput> input)
    : CorruptIndexException(message, input, nullptr)
{
}

CorruptIndexException::CorruptIndexException(const wstring &message,
                                             shared_ptr<DataOutput> output)
    : CorruptIndexException(message, output, nullptr)
{
}

CorruptIndexException::CorruptIndexException(const wstring &message,
                                             shared_ptr<DataInput> input,
                                             runtime_error cause)
    : CorruptIndexException(message, Objects->toString(input), cause)
{
}

CorruptIndexException::CorruptIndexException(const wstring &message,
                                             shared_ptr<DataOutput> output,
                                             runtime_error cause)
    : CorruptIndexException(message, Objects->toString(output), cause)
{
}

CorruptIndexException::CorruptIndexException(const wstring &message,
                                             const wstring &resourceDescription)
    : CorruptIndexException(message, resourceDescription, nullptr)
{
}

CorruptIndexException::CorruptIndexException(const wstring &message,
                                             const wstring &resourceDescription,
                                             runtime_error cause)
    : java::io::IOException(Objects->toString(message) + L" (resource=" +
                                resourceDescription + L")",
                            cause),
      message(message), resourceDescription(resourceDescription)
{
}

wstring CorruptIndexException::getResourceDescription()
{
  return resourceDescription;
}

wstring CorruptIndexException::getOriginalMessage() { return message; }
} // namespace org::apache::lucene::index