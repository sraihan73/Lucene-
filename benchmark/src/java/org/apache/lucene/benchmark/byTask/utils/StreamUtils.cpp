using namespace std;

#include "StreamUtils.h"

namespace org::apache::lucene::benchmark::byTask::utils
{
using org::apache::commons::compress::compressors::CompressorException;
using org::apache::commons::compress::compressors::CompressorStreamFactory;

Type Type::BZIP2(L"BZIP2", InnerEnum::BZIP2,
                 org::apache::commons::compress::compressors::
                     CompressorStreamFactory::BZIP2);
Type Type::GZIP(
    L"GZIP", InnerEnum::GZIP,
    org::apache::commons::compress::compressors::CompressorStreamFactory::GZIP);
Type Type::PLAIN(L"PLAIN", InnerEnum::PLAIN, nullptr);

deque<Type> Type::valueList;

Type::StaticConstructor::StaticConstructor()
{
  valueList.push_back(BZIP2);
  valueList.push_back(GZIP);
  valueList.push_back(PLAIN);
}

Type::StaticConstructor Type::staticConstructor;
int Type::nextOrdinal = 0;

StreamUtils::Type::Type(const wstring &name, InnerEnum innerEnum,
                        shared_ptr<StreamUtils> outerInstance,
                        const wstring &csfType)
    : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
  this->outerInstance = outerInstance;
  this->csfType = csfType;
}

shared_ptr<java::io::InputStream> StreamUtils::Type::inputStream(
    shared_ptr<java::io::InputStream> in_) 
{
  try {
    return outerInstance->csfType == L""
               ? in_
               : (make_shared<CompressorStreamFactory>())
                     ->createCompressorInputStream(outerInstance->csfType, in_);
  } catch (const CompressorException &e) {
    throw make_shared<IOException>(e->getMessage(), e);
  }
}

shared_ptr<java::io::OutputStream> StreamUtils::Type::outputStream(
    shared_ptr<java::io::OutputStream> os) 
{
  try {
    return outerInstance->csfType == L""
               ? os
               : (make_shared<CompressorStreamFactory>())
                     ->createCompressorOutputStream(outerInstance->csfType, os);
  } catch (const CompressorException &e) {
    throw make_shared<IOException>(e->getMessage(), e);
  }
}

bool Type::operator==(const Type &other)
{
  return this->ordinalValue == other.ordinalValue;
}

bool Type::operator!=(const Type &other)
{
  return this->ordinalValue != other.ordinalValue;
}

deque<Type> Type::values() { return valueList; }

int Type::ordinal() { return ordinalValue; }

wstring Type::toString() { return nameValue; }

Type Type::valueOf(const wstring &name)
{
  for (auto enumInstance : Type::valueList) {
    if (enumInstance.nameValue == name) {
      return enumInstance;
    }
  }
}

const unordered_map<wstring, Type> StreamUtils::extensionToType =
    unordered_map<wstring, Type>();

StreamUtils::StaticConstructor::StaticConstructor()
{
  // these in are lower case, we will lower case at the test as well
  extensionToType.emplace(L".bz2", Type::BZIP2);
  extensionToType.emplace(L".bzip", Type::BZIP2);
  extensionToType.emplace(L".gz", Type::GZIP);
  extensionToType.emplace(L".gzip", Type::GZIP);
}

StreamUtils::StaticConstructor StreamUtils::staticConstructor;

shared_ptr<InputStream>
StreamUtils::inputStream(shared_ptr<Path> file) 
{
  // First, create a FileInputStream, as this will be required by all types.
  // Wrap with BufferedInputStream for better performance
  shared_ptr<InputStream> in_ = make_shared<BufferedInputStream>(
      Files::newInputStream(file), BUFFER_SIZE);
  return fileType(file).inputStream(in_);
}

StreamUtils::Type StreamUtils::fileType(shared_ptr<Path> file)
{
  Type type = nullptr;
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring fileName = file->getFileName()->toString();
  int idx = (int)fileName.rfind(L'.');
  if (idx != -1) {
    type = extensionToType[fileName.substr(idx)->toLowerCase(Locale::ROOT)];
  }
  return type == nullptr ? Type::PLAIN : type;
}

shared_ptr<OutputStream>
StreamUtils::outputStream(shared_ptr<Path> file) 
{
  // First, create a FileInputStream, as this will be required by all types.
  // Wrap with BufferedInputStream for better performance
  shared_ptr<OutputStream> os = make_shared<BufferedOutputStream>(
      Files::newOutputStream(file), BUFFER_SIZE);
  return fileType(file).outputStream(os);
}
} // namespace org::apache::lucene::benchmark::byTask::utils