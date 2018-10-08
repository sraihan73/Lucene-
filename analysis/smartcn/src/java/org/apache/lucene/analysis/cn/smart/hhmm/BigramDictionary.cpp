using namespace std;

#include "BigramDictionary.h"
#include "../AnalyzerProfile.h"

namespace org::apache::lucene::analysis::cn::smart::hhmm
{
using AnalyzerProfile =
    org::apache::lucene::analysis::cn::smart::AnalyzerProfile;

BigramDictionary::BigramDictionary() {}

shared_ptr<BigramDictionary> BigramDictionary::singleInstance;

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<BigramDictionary> BigramDictionary::getInstance()
{
  if (singleInstance == nullptr) {
    singleInstance = make_shared<BigramDictionary>();
    try {
      singleInstance->load();
    } catch (const IOException &e) {
      wstring dictRoot = AnalyzerProfile::ANALYSIS_DATA_DIR;
      try {
        singleInstance->load(dictRoot);
      } catch (const IOException &ioe) {
        throw runtime_error(ioe);
      }
    } catch (const ClassNotFoundException &e) {
      throw runtime_error(e);
    }
  }
  return singleInstance;
}

bool BigramDictionary::loadFromObj(shared_ptr<Path> serialObj)
{
  try {
    loadFromInputStream(Files::newInputStream(serialObj));
    return true;
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

void BigramDictionary::loadFromInputStream(
    shared_ptr<InputStream>
        serialObjectInputStream) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.ObjectInputStream input = new
  // java.io.ObjectInputStream(serialObjectInputStream))
  {
    java::io::ObjectInputStream input =
        java::io::ObjectInputStream(serialObjectInputStream);
    bigramHashTable = static_cast<std::deque<int64_t>>(input.readObject());
    frequencyTable = static_cast<std::deque<int>>(input.readObject());
    // log.info("load bigram dict from serialization.");
  }
}

void BigramDictionary::saveToObj(shared_ptr<Path> serialObj) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.ObjectOutputStream output = new
  // java.io.ObjectOutputStream(java.nio.file.Files.newOutputStream(serialObj)))
  {
    java::io::ObjectOutputStream output = java::io::ObjectOutputStream(
        java::nio::file::Files::newOutputStream(serialObj));
    output.writeObject(bigramHashTable);
    output.writeObject(frequencyTable);
    // log.info("serialize bigram dict.");
  }
}

void BigramDictionary::load() 
{
  shared_ptr<InputStream> input =
      this->getClass().getResourceAsStream(L"bigramdict.mem");
  loadFromInputStream(input);
}

void BigramDictionary::load(const wstring &dictRoot) 
{
  wstring bigramDictPath = dictRoot + L"/bigramdict.dct";

  shared_ptr<Path> serialObj = Paths->get(dictRoot + L"/bigramdict.mem");

  if (Files::exists(serialObj) && loadFromObj(serialObj)) {

  } else {
    try {
      bigramHashTable = std::deque<int64_t>(PRIME_BIGRAM_LENGTH);
      frequencyTable = std::deque<int>(PRIME_BIGRAM_LENGTH);
      for (int i = 0; i < PRIME_BIGRAM_LENGTH; i++) {
        // it is possible for a value to hash to 0, but the probability is
        // extremely low
        bigramHashTable[i] = 0;
        frequencyTable[i] = 0;
      }
      loadFromFile(bigramDictPath);
    } catch (const IOException &e) {
      throw runtime_error(e->getMessage());
    }
    saveToObj(serialObj);
  }
}

void BigramDictionary::loadFromFile(const wstring &dctFilePath) throw(
    IOException)
{

  int i, cnt, length, total = 0;
  // The file only counted 6763 Chinese characters plus 5 reserved slots
  // 3756~3760. The 3756th is used (as a header) to store information.
  std::deque<int> buffer(3);
  std::deque<char> intBuffer(4);
  wstring tmpword;
  shared_ptr<DataInputStream> dctFile = make_shared<DataInputStream>(
      Files::newInputStream(Paths->get(dctFilePath)));

  // GB2312 characters 0 - 6768
  for (i = GB2312_FIRST_CHAR; i < GB2312_FIRST_CHAR + CHAR_NUM_IN_FILE; i++) {
    wstring currentStr = getCCByGB2312Id(i);
    // if (i == 5231)
    // System.out.println(i);

    dctFile->read(intBuffer);
    // the dictionary was developed for C, and byte order must be converted to
    // work with Java
    cnt = ByteBuffer::wrap(intBuffer).order(ByteOrder::LITTLE_ENDIAN).getInt();
    if (cnt <= 0) {
      continue;
    }
    total += cnt;
    int j = 0;
    while (j < cnt) {
      dctFile->read(intBuffer);
      buffer[0] = ByteBuffer::wrap(intBuffer)
                      .order(ByteOrder::LITTLE_ENDIAN)
                      .getInt(); // frequency
      dctFile->read(intBuffer);
      buffer[1] = ByteBuffer::wrap(intBuffer)
                      .order(ByteOrder::LITTLE_ENDIAN)
                      .getInt(); // length
      dctFile->read(intBuffer);
      // buffer[2] = ByteBuffer.wrap(intBuffer).order(
      // ByteOrder.LITTLE_ENDIAN).getInt();// handle

      length = buffer[1];
      if (length > 0) {
        std::deque<char> lchBuffer(length);
        dctFile->read(lchBuffer);
        tmpword = wstring(lchBuffer, L"GB2312");
        if (i != 3755 + GB2312_FIRST_CHAR) {
          tmpword = currentStr + tmpword;
        }
        std::deque<wchar_t> carray = tmpword.toCharArray();
        int64_t hashId = hash1(carray);
        int index = getAvaliableIndex(hashId, carray);
        if (index != -1) {
          if (bigramHashTable[index] == 0) {
            bigramHashTable[index] = hashId;
            // bigramStringTable[index] = tmpword;
          }
          frequencyTable[index] += buffer[0];
        }
      }
      j++;
    }
  }
  dctFile->close();
  // log.info("load dictionary done! " + dctFilePath + " total:" + total);
}

int BigramDictionary::getAvaliableIndex(int64_t hashId,
                                        std::deque<wchar_t> &carray)
{
  int hash1 = static_cast<int>(hashId % PRIME_BIGRAM_LENGTH);
  int hash2 = this->hash2(carray) % PRIME_BIGRAM_LENGTH;
  if (hash1 < 0) {
    hash1 = PRIME_BIGRAM_LENGTH + hash1;
  }
  if (hash2 < 0) {
    hash2 = PRIME_BIGRAM_LENGTH + hash2;
  }
  int index = hash1;
  int i = 1;
  while (bigramHashTable[index] != 0 && bigramHashTable[index] != hashId &&
         i < PRIME_BIGRAM_LENGTH) {
    index = (hash1 + i * hash2) % PRIME_BIGRAM_LENGTH;
    i++;
  }
  // System.out.println(i - 1);

  if (i < PRIME_BIGRAM_LENGTH &&
      (bigramHashTable[index] == 0 || bigramHashTable[index] == hashId)) {
    return index;
  } else {
    return -1;
  }
}

int BigramDictionary::getBigramItemIndex(std::deque<wchar_t> &carray)
{
  int64_t hashId = hash1(carray);
  int hash1 = static_cast<int>(hashId % PRIME_BIGRAM_LENGTH);
  int hash2 = this->hash2(carray) % PRIME_BIGRAM_LENGTH;
  if (hash1 < 0) {
    hash1 = PRIME_BIGRAM_LENGTH + hash1;
  }
  if (hash2 < 0) {
    hash2 = PRIME_BIGRAM_LENGTH + hash2;
  }
  int index = hash1;
  int i = 1;
  repeat++;
  while (bigramHashTable[index] != 0 && bigramHashTable[index] != hashId &&
         i < PRIME_BIGRAM_LENGTH) {
    index = (hash1 + i * hash2) % PRIME_BIGRAM_LENGTH;
    i++;
    repeat++;
    if (i > max) {
      max = i;
    }
  }
  // System.out.println(i - 1);

  if (i < PRIME_BIGRAM_LENGTH && bigramHashTable[index] == hashId) {
    return index;
  } else {
    return -1;
  }
}

int BigramDictionary::getFrequency(std::deque<wchar_t> &carray)
{
  int index = getBigramItemIndex(carray);
  if (index != -1) {
    return frequencyTable[index];
  }
  return 0;
}
} // namespace org::apache::lucene::analysis::cn::smart::hhmm