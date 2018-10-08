using namespace std;

#include "PortugueseStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    PortugueseStemmer::methodObject =
        java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    PortugueseStemmer::a_0 = {make_shared<org::tartarus::snowball::Among>(
                                  L"", -1, 3, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"\u00E3", 0, 1, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"\u00F5", 0, 2, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    PortugueseStemmer::a_1 = {make_shared<org::tartarus::snowball::Among>(
                                  L"", -1, 3, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"a~", 0, 1, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"o~", 0, 2, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    PortugueseStemmer::a_2 = {make_shared<org::tartarus::snowball::Among>(
                                  L"ic", -1, -1, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"ad", -1, -1, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"os", -1, -1, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"iv", -1, 1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    PortugueseStemmer::a_3 = {make_shared<org::tartarus::snowball::Among>(
                                  L"ante", -1, 1, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"avel", -1, 1, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"\u00EDvel", -1, 1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    PortugueseStemmer::a_4 = {make_shared<org::tartarus::snowball::Among>(
                                  L"ic", -1, 1, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"abil", -1, 1, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"iv", -1, 1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    PortugueseStemmer::a_5 = {
        make_shared<org::tartarus::snowball::Among>(L"ica", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E2ncia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EAncia", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ira", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"adora", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"osa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ista", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iva", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eza", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"log\u00EDa", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"idade", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ante", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mente", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"amente", 12, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1vel", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDvel", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uci\u00F3n", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ico", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ismo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oso", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"amento", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"imento", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivo", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"a\u00E7a~o", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ador", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icas", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EAncias", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iras", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"adoras", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"osas", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"istas", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivas", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ezas", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"log\u00EDas", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"idades", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uciones", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"adores", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"antes", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"a\u00E7o~es", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ismos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"osos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"amentos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"imentos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivos", -1, 8, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    PortugueseStemmer::a_6 = {
        make_shared<org::tartarus::snowball::Among>(L"ada", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ida", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aria", 2, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eria", 2, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iria", 2, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ara", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"era", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ira", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ava", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asse", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esse", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isse", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aste", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"este", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iste", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ei", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arei", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erei", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irei", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"am", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iam", 20, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ariam", 21, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eriam", 21, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iriam", 21, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aram", 20, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eram", 20, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iram", 20, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"avam", 20, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"em", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arem", 29, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erem", 29, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irem", 29, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assem", 29, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"essem", 29, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issem", 29, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ado", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ido", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ando", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"endo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"indo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ara~o", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"era~o", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ira~o", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"as", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"adas", 47, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"idas", 47, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ias", 47, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arias", 50, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erias", 50, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irias", 50, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aras", 47, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eras", 47, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iras", 47, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"avas", 47, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"es", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ardes", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erdes", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irdes", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ares", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eres", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ires", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asses", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esses", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isses", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"astes", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"estes", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"istes", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"is", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ais", 71, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eis", 71, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"areis", 73, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ereis", 73, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ireis", 73, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1reis", 73, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9reis", 73, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDreis", 73, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1sseis", 73, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9sseis", 73, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDsseis", 73, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1veis", 73, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDeis", 73, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00EDeis", 84, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00EDeis", 84, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00EDeis", 84, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ados", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"idos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"amos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1ramos", 90, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9ramos", 90, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDramos", 90, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1vamos", 90, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDamos", 90, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00EDamos", 95, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00EDamos", 95, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00EDamos", 95, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"emos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aremos", 99, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eremos", 99, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iremos", 99, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1ssemos", 99, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EAssemos", 99, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDssemos", 99, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"imos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"armos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ermos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irmos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1mos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00E1s", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00E1s", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00E1s", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ou", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00E1", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00E1", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00E1", -1, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    PortugueseStemmer::a_7 = {
        make_shared<org::tartarus::snowball::Among>(L"a", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"o", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"os", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00ED", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F3", -1, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    PortugueseStemmer::a_8 = {make_shared<org::tartarus::snowball::Among>(
                                  L"e", -1, 1, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"\u00E7", -1, 2, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"\u00E9", -1, 1, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"\u00EA", -1, 1, L"", methodObject)};
std::deque<wchar_t> const PortugueseStemmer::g_v = {
    17, 65, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 19, 12, 2};

void PortugueseStemmer::copy_from(shared_ptr<PortugueseStemmer> other)
{
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  I_pV = other->I_pV;
  SnowballProgram::copy_from(other);
}

bool PortugueseStemmer::r_prelude()
{
  int among_var;
  int v_1;
  // repeat, line 36
  while (true) {
    v_1 = cursor;
    do {
      // (, line 36
      // [, line 37
      bra = cursor;
      // substring, line 37
      among_var = find_among(a_0, 3);
      if (among_var == 0) {
        goto lab1Break;
      }
      // ], line 37
      ket = cursor;
      switch (among_var) {
      case 0:
        goto lab1Break;
      case 1:
        // (, line 38
        // <-, line 38
        slice_from(L"a~");
        break;
      case 2:
        // (, line 39
        // <-, line 39
        slice_from(L"o~");
        break;
      case 3:
        // (, line 40
        // next, line 40
        if (cursor >= limit) {
          goto lab1Break;
        }
        cursor++;
        break;
      }
      goto replab0Continue;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = v_1;
    goto replab0Break;
  replab0Continue:;
  }
replab0Break:
  return true;
}

bool PortugueseStemmer::r_mark_regions()
{
  int v_1;
  int v_2;
  int v_3;
  int v_6;
  int v_8;
  // (, line 44
  I_pV = limit;
  I_p1 = limit;
  I_p2 = limit;
  // do, line 50
  v_1 = cursor;
  do {
    // (, line 50
    // or, line 52
    do {
      v_2 = cursor;
      do {
        // (, line 51
        if (!(in_grouping(g_v, 97, 250))) {
          goto lab2Break;
        }
        // or, line 51
        do {
          v_3 = cursor;
          do {
            // (, line 51
            if (!(out_grouping(g_v, 97, 250))) {
              goto lab4Break;
            }
            // gopast, line 51
            while (true) {
              do {
                if (!(in_grouping(g_v, 97, 250))) {
                  goto lab6Break;
                }
                goto golab5Break;
              } while (false);
            lab6Continue:;
            lab6Break:
              if (cursor >= limit) {
                goto lab4Break;
              }
              cursor++;
            golab5Continue:;
            }
          golab5Break:
            goto lab3Break;
          } while (false);
        lab4Continue:;
        lab4Break:
          cursor = v_3;
          // (, line 51
          if (!(in_grouping(g_v, 97, 250))) {
            goto lab2Break;
          }
          // gopast, line 51
          while (true) {
            do {
              if (!(out_grouping(g_v, 97, 250))) {
                goto lab8Break;
              }
              goto golab7Break;
            } while (false);
          lab8Continue:;
          lab8Break:
            if (cursor >= limit) {
              goto lab2Break;
            }
            cursor++;
          golab7Continue:;
          }
        golab7Break:;
        } while (false);
      lab3Continue:;
      lab3Break:
        goto lab1Break;
      } while (false);
    lab2Continue:;
    lab2Break:
      cursor = v_2;
      // (, line 53
      if (!(out_grouping(g_v, 97, 250))) {
        goto lab0Break;
      }
      // or, line 53
      do {
        v_6 = cursor;
        do {
          // (, line 53
          if (!(out_grouping(g_v, 97, 250))) {
            goto lab10Break;
          }
          // gopast, line 53
          while (true) {
            do {
              if (!(in_grouping(g_v, 97, 250))) {
                goto lab12Break;
              }
              goto golab11Break;
            } while (false);
          lab12Continue:;
          lab12Break:
            if (cursor >= limit) {
              goto lab10Break;
            }
            cursor++;
          golab11Continue:;
          }
        golab11Break:
          goto lab9Break;
        } while (false);
      lab10Continue:;
      lab10Break:
        cursor = v_6;
        // (, line 53
        if (!(in_grouping(g_v, 97, 250))) {
          goto lab0Break;
        }
        // next, line 53
        if (cursor >= limit) {
          goto lab0Break;
        }
        cursor++;
      } while (false);
    lab9Continue:;
    lab9Break:;
    } while (false);
  lab1Continue:;
  lab1Break:
    // setmark pV, line 54
    I_pV = cursor;
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 56
  v_8 = cursor;
  do {
    // (, line 56
    // gopast, line 57
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 250))) {
          goto lab15Break;
        }
        goto golab14Break;
      } while (false);
    lab15Continue:;
    lab15Break:
      if (cursor >= limit) {
        goto lab13Break;
      }
      cursor++;
    golab14Continue:;
    }
  golab14Break:
    // gopast, line 57
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 250))) {
          goto lab17Break;
        }
        goto golab16Break;
      } while (false);
    lab17Continue:;
    lab17Break:
      if (cursor >= limit) {
        goto lab13Break;
      }
      cursor++;
    golab16Continue:;
    }
  golab16Break:
    // setmark p1, line 57
    I_p1 = cursor;
    // gopast, line 58
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 250))) {
          goto lab19Break;
        }
        goto golab18Break;
      } while (false);
    lab19Continue:;
    lab19Break:
      if (cursor >= limit) {
        goto lab13Break;
      }
      cursor++;
    golab18Continue:;
    }
  golab18Break:
    // gopast, line 58
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 250))) {
          goto lab21Break;
        }
        goto golab20Break;
      } while (false);
    lab21Continue:;
    lab21Break:
      if (cursor >= limit) {
        goto lab13Break;
      }
      cursor++;
    golab20Continue:;
    }
  golab20Break:
    // setmark p2, line 58
    I_p2 = cursor;
  } while (false);
lab13Continue:;
lab13Break:
  cursor = v_8;
  return true;
}

bool PortugueseStemmer::r_postlude()
{
  int among_var;
  int v_1;
  // repeat, line 62
  while (true) {
    v_1 = cursor;
    do {
      // (, line 62
      // [, line 63
      bra = cursor;
      // substring, line 63
      among_var = find_among(a_1, 3);
      if (among_var == 0) {
        goto lab1Break;
      }
      // ], line 63
      ket = cursor;
      switch (among_var) {
      case 0:
        goto lab1Break;
      case 1:
        // (, line 64
        // <-, line 64
        slice_from(L"\u00E3");
        break;
      case 2:
        // (, line 65
        // <-, line 65
        slice_from(L"\u00F5");
        break;
      case 3:
        // (, line 66
        // next, line 66
        if (cursor >= limit) {
          goto lab1Break;
        }
        cursor++;
        break;
      }
      goto replab0Continue;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = v_1;
    goto replab0Break;
  replab0Continue:;
  }
replab0Break:
  return true;
}

bool PortugueseStemmer::r_RV()
{
  if (!(I_pV <= cursor)) {
    return false;
  }
  return true;
}

bool PortugueseStemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool PortugueseStemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool PortugueseStemmer::r_standard_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  // (, line 76
  // [, line 77
  ket = cursor;
  // substring, line 77
  among_var = find_among_b(a_5, 45);
  if (among_var == 0) {
    return false;
  }
  // ], line 77
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 92
    // call R2, line 93
    if (!r_R2()) {
      return false;
    }
    // delete, line 93
    slice_del();
    break;
  case 2:
    // (, line 97
    // call R2, line 98
    if (!r_R2()) {
      return false;
    }
    // <-, line 98
    slice_from(L"log");
    break;
  case 3:
    // (, line 101
    // call R2, line 102
    if (!r_R2()) {
      return false;
    }
    // <-, line 102
    slice_from(L"u");
    break;
  case 4:
    // (, line 105
    // call R2, line 106
    if (!r_R2()) {
      return false;
    }
    // <-, line 106
    slice_from(L"ente");
    break;
  case 5:
    // (, line 109
    // call R1, line 110
    if (!r_R1()) {
      return false;
    }
    // delete, line 110
    slice_del();
    // try, line 111
    v_1 = limit - cursor;
    do {
      // (, line 111
      // [, line 112
      ket = cursor;
      // substring, line 112
      among_var = find_among_b(a_2, 4);
      if (among_var == 0) {
        cursor = limit - v_1;
        goto lab0Break;
      }
      // ], line 112
      bra = cursor;
      // call R2, line 112
      if (!r_R2()) {
        cursor = limit - v_1;
        goto lab0Break;
      }
      // delete, line 112
      slice_del();
      switch (among_var) {
      case 0:
        cursor = limit - v_1;
        goto lab0Break;
      case 1:
        // (, line 113
        // [, line 113
        ket = cursor;
        // literal, line 113
        if (!(eq_s_b(2, L"at"))) {
          cursor = limit - v_1;
          goto lab0Break;
        }
        // ], line 113
        bra = cursor;
        // call R2, line 113
        if (!r_R2()) {
          cursor = limit - v_1;
          goto lab0Break;
        }
        // delete, line 113
        slice_del();
        break;
      }
    } while (false);
  lab0Continue:;
  lab0Break:
    break;
  case 6:
    // (, line 121
    // call R2, line 122
    if (!r_R2()) {
      return false;
    }
    // delete, line 122
    slice_del();
    // try, line 123
    v_2 = limit - cursor;
    do {
      // (, line 123
      // [, line 124
      ket = cursor;
      // substring, line 124
      among_var = find_among_b(a_3, 3);
      if (among_var == 0) {
        cursor = limit - v_2;
        goto lab1Break;
      }
      // ], line 124
      bra = cursor;
      switch (among_var) {
      case 0:
        cursor = limit - v_2;
        goto lab1Break;
      case 1:
        // (, line 127
        // call R2, line 127
        if (!r_R2()) {
          cursor = limit - v_2;
          goto lab1Break;
        }
        // delete, line 127
        slice_del();
        break;
      }
    } while (false);
  lab1Continue:;
  lab1Break:
    break;
  case 7:
    // (, line 133
    // call R2, line 134
    if (!r_R2()) {
      return false;
    }
    // delete, line 134
    slice_del();
    // try, line 135
    v_3 = limit - cursor;
    do {
      // (, line 135
      // [, line 136
      ket = cursor;
      // substring, line 136
      among_var = find_among_b(a_4, 3);
      if (among_var == 0) {
        cursor = limit - v_3;
        goto lab2Break;
      }
      // ], line 136
      bra = cursor;
      switch (among_var) {
      case 0:
        cursor = limit - v_3;
        goto lab2Break;
      case 1:
        // (, line 139
        // call R2, line 139
        if (!r_R2()) {
          cursor = limit - v_3;
          goto lab2Break;
        }
        // delete, line 139
        slice_del();
        break;
      }
    } while (false);
  lab2Continue:;
  lab2Break:
    break;
  case 8:
    // (, line 145
    // call R2, line 146
    if (!r_R2()) {
      return false;
    }
    // delete, line 146
    slice_del();
    // try, line 147
    v_4 = limit - cursor;
    do {
      // (, line 147
      // [, line 148
      ket = cursor;
      // literal, line 148
      if (!(eq_s_b(2, L"at"))) {
        cursor = limit - v_4;
        goto lab3Break;
      }
      // ], line 148
      bra = cursor;
      // call R2, line 148
      if (!r_R2()) {
        cursor = limit - v_4;
        goto lab3Break;
      }
      // delete, line 148
      slice_del();
    } while (false);
  lab3Continue:;
  lab3Break:
    break;
  case 9:
    // (, line 152
    // call RV, line 153
    if (!r_RV()) {
      return false;
    }
    // literal, line 153
    if (!(eq_s_b(1, L"e"))) {
      return false;
    }
    // <-, line 154
    slice_from(L"ir");
    break;
  }
  return true;
}

bool PortugueseStemmer::r_verb_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  // setlimit, line 159
  v_1 = limit - cursor;
  // tomark, line 159
  if (cursor < I_pV) {
    return false;
  }
  cursor = I_pV;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 159
  // [, line 160
  ket = cursor;
  // substring, line 160
  among_var = find_among_b(a_6, 120);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 160
  bra = cursor;
  switch (among_var) {
  case 0:
    limit_backward = v_2;
    return false;
  case 1:
    // (, line 179
    // delete, line 179
    slice_del();
    break;
  }
  limit_backward = v_2;
  return true;
}

bool PortugueseStemmer::r_residual_suffix()
{
  int among_var;
  // (, line 183
  // [, line 184
  ket = cursor;
  // substring, line 184
  among_var = find_among_b(a_7, 7);
  if (among_var == 0) {
    return false;
  }
  // ], line 184
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 187
    // call RV, line 187
    if (!r_RV()) {
      return false;
    }
    // delete, line 187
    slice_del();
    break;
  }
  return true;
}

bool PortugueseStemmer::r_residual_form()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  // (, line 191
  // [, line 192
  ket = cursor;
  // substring, line 192
  among_var = find_among_b(a_8, 4);
  if (among_var == 0) {
    return false;
  }
  // ], line 192
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 194
    // call RV, line 194
    if (!r_RV()) {
      return false;
    }
    // delete, line 194
    slice_del();
    // [, line 194
    ket = cursor;
    // or, line 194
    do {
      v_1 = limit - cursor;
      do {
        // (, line 194
        // literal, line 194
        if (!(eq_s_b(1, L"u"))) {
          goto lab1Break;
        }
        // ], line 194
        bra = cursor;
        // test, line 194
        v_2 = limit - cursor;
        // literal, line 194
        if (!(eq_s_b(1, L"g"))) {
          goto lab1Break;
        }
        cursor = limit - v_2;
        goto lab0Break;
      } while (false);
    lab1Continue:;
    lab1Break:
      cursor = limit - v_1;
      // (, line 195
      // literal, line 195
      if (!(eq_s_b(1, L"i"))) {
        return false;
      }
      // ], line 195
      bra = cursor;
      // test, line 195
      v_3 = limit - cursor;
      // literal, line 195
      if (!(eq_s_b(1, L"c"))) {
        return false;
      }
      cursor = limit - v_3;
    } while (false);
  lab0Continue:;
  lab0Break:
    // call RV, line 195
    if (!r_RV()) {
      return false;
    }
    // delete, line 195
    slice_del();
    break;
  case 2:
    // (, line 196
    // <-, line 196
    slice_from(L"c");
    break;
  }
  return true;
}

bool PortugueseStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  int v_8;
  int v_9;
  int v_10;
  // (, line 201
  // do, line 202
  v_1 = cursor;
  do {
    // call prelude, line 202
    if (!r_prelude()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 203
  v_2 = cursor;
  do {
    // call mark_regions, line 203
    if (!r_mark_regions()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = v_2;
  // backwards, line 204
  limit_backward = cursor;
  cursor = limit;
  // (, line 204
  // do, line 205
  v_3 = limit - cursor;
  do {
    // (, line 205
    // or, line 209
    do {
      v_4 = limit - cursor;
      do {
        // (, line 206
        // and, line 207
        v_5 = limit - cursor;
        // (, line 206
        // or, line 206
        do {
          v_6 = limit - cursor;
          do {
            // call standard_suffix, line 206
            if (!r_standard_suffix()) {
              goto lab6Break;
            }
            goto lab5Break;
          } while (false);
        lab6Continue:;
        lab6Break:
          cursor = limit - v_6;
          // call verb_suffix, line 206
          if (!r_verb_suffix()) {
            goto lab4Break;
          }
        } while (false);
      lab5Continue:;
      lab5Break:
        cursor = limit - v_5;
        // do, line 207
        v_7 = limit - cursor;
        do {
          // (, line 207
          // [, line 207
          ket = cursor;
          // literal, line 207
          if (!(eq_s_b(1, L"i"))) {
            goto lab7Break;
          }
          // ], line 207
          bra = cursor;
          // test, line 207
          v_8 = limit - cursor;
          // literal, line 207
          if (!(eq_s_b(1, L"c"))) {
            goto lab7Break;
          }
          cursor = limit - v_8;
          // call RV, line 207
          if (!r_RV()) {
            goto lab7Break;
          }
          // delete, line 207
          slice_del();
        } while (false);
      lab7Continue:;
      lab7Break:
        cursor = limit - v_7;
        goto lab3Break;
      } while (false);
    lab4Continue:;
    lab4Break:
      cursor = limit - v_4;
      // call residual_suffix, line 209
      if (!r_residual_suffix()) {
        goto lab2Break;
      }
    } while (false);
  lab3Continue:;
  lab3Break:;
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  // do, line 211
  v_9 = limit - cursor;
  do {
    // call residual_form, line 211
    if (!r_residual_form()) {
      goto lab8Break;
    }
  } while (false);
lab8Continue:;
lab8Break:
  cursor = limit - v_9;
  cursor = limit_backward; // do, line 213
  v_10 = cursor;
  do {
    // call postlude, line 213
    if (!r_postlude()) {
      goto lab9Break;
    }
  } while (false);
lab9Continue:;
lab9Break:
  cursor = v_10;
  return true;
}

bool PortugueseStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<PortugueseStemmer>(o) != nullptr;
}

int PortugueseStemmer::hashCode()
{
  return PortugueseStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext