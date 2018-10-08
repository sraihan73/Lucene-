using namespace std;

#include "IndexNotFoundException.h"

namespace org::apache::lucene::index
{

IndexNotFoundException::IndexNotFoundException(const wstring &msg)
    : java::io::FileNotFoundException(msg)
{
}
} // namespace org::apache::lucene::index