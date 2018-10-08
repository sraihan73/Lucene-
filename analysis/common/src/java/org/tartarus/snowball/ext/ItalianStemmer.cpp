using namespace std;

#include "ItalianStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    ItalianStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    ItalianStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"qu", 0, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9", 0, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00ED", 0, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F3", 0, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00FA", 0, 5, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    ItalianStemmer::a_1 = {make_shared<org::tartarus::snowball::Among>(
                               L"", -1, 3, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"I", 0, 1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"U", 0, 2, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    ItalianStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"la", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"cela", 0, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gliela", 0, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mela", 0, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tela", 0, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"vela", 0, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"le", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"cele", 6, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gliele", 6, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mele", 6, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tele", 6, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"vele", 6, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ne", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"cene", 12, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gliene", 12, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mene", 12, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sene", 12, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tene", 12, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"vene", 12, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ci", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"li", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"celi", 20, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"glieli", 20, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"meli", 20, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"teli", 20, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"veli", 20, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gli", 20, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mi", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"si", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ti", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"vi", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lo", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"celo", 31, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"glielo", 31, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"melo", 31, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"telo", 31, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"velo", 31, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    ItalianStemmer::a_3 = {
        make_shared<org::tartarus::snowball::Among>(L"ando", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"endo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir", -1, 2, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    ItalianStemmer::a_4 = {make_shared<org::tartarus::snowball::Among>(
                               L"ic", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"abil", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"os", -1, -1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"iv", -1, 1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    ItalianStemmer::a_5 = {make_shared<org::tartarus::snowball::Among>(
                               L"ic", -1, 1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"abil", -1, 1, L"", methodObject),
                           make_shared<org::tartarus::snowball::Among>(
                               L"iv", -1, 1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    ItalianStemmer::a_6 = {
        make_shared<org::tartarus::snowball::Among>(L"ica", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"logia", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"osa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ista", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iva", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"anza", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enza", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ice", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atrice", 7, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iche", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"logie", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abile", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ibile", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"usione", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"azione", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uzione", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atore", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ose", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ante", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mente", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"amente", 19, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iste", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ive", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"anze", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enze", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ici", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atrici", 25, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ichi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abili", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ibili", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ismi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"usioni", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"azioni", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uzioni", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atori", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"osi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"anti", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"amenti", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"imenti", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isti", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivi", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ico", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ismo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oso", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"amento", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"imento", -1, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivo", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"it\u00E0", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ist\u00E0", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ist\u00E8", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ist\u00EC", -1, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    ItalianStemmer::a_7 = {
        make_shared<org::tartarus::snowball::Among>(L"isca", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enda", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ata", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ita", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uta", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ava", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eva", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iva", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erebbe", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irebbe", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isce", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ende", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"are", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ere", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ire", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asse", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ate", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"avate", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"evate", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivate", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ete", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erete", 20, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irete", 20, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ite", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ereste", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ireste", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ute", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erai", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irai", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isci", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"endi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erei", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irei", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ati", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iti", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eresti", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iresti", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uti", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"avi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"evi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isco", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ando", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"endo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"Yamo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iamo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"avamo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"evamo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivamo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eremo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iremo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assimo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ammo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"emmo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eremmo", 54, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iremmo", 54, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"immo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ano", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iscano", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"avano", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"evano", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivano", 58, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eranno", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iranno", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ono", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iscono", 65, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arono", 65, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erono", 65, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irono", 65, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erebbero", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irebbero", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assero", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"essero", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issero", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ato", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ito", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uto", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"avo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"evo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00E0", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00E0", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00F2", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00F2", -1, 1, L"",
                                                    methodObject)};
std::deque<wchar_t> const ItalianStemmer::g_v = {
    17, 65, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 128, 8, 2, 1};
std::deque<wchar_t> const ItalianStemmer::g_AEIO = {
    17, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 128, 8, 2};
std::deque<wchar_t> const ItalianStemmer::g_CG = {17};

void ItalianStemmer::copy_from(shared_ptr<ItalianStemmer> other)
{
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  I_pV = other->I_pV;
  SnowballProgram::copy_from(other);
}

bool ItalianStemmer::r_prelude()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  // (, line 34
  // test, line 35
  v_1 = cursor;
  // repeat, line 35
  while (true) {
    v_2 = cursor;
    do {
      // (, line 35
      // [, line 36
      bra = cursor;
      // substring, line 36
      among_var = find_among(a_0, 7);
      if (among_var == 0) {
        goto lab1Break;
      }
      // ], line 36
      ket = cursor;
      switch (among_var) {
      case 0:
        goto lab1Break;
      case 1:
        // (, line 37
        // <-, line 37
        slice_from(L"\u00E0");
        break;
      case 2:
        // (, line 38
        // <-, line 38
        slice_from(L"\u00E8");
        break;
      case 3:
        // (, line 39
        // <-, line 39
        slice_from(L"\u00EC");
        break;
      case 4:
        // (, line 40
        // <-, line 40
        slice_from(L"\u00F2");
        break;
      case 5:
        // (, line 41
        // <-, line 41
        slice_from(L"\u00F9");
        break;
      case 6:
        // (, line 42
        // <-, line 42
        slice_from(L"qU");
        break;
      case 7:
        // (, line 43
        // next, line 43
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
    cursor = v_2;
    goto replab0Break;
  replab0Continue:;
  }
replab0Break:
  cursor = v_1;
  // repeat, line 46
  while (true) {
    v_3 = cursor;
    do {
      // goto, line 46
      while (true) {
        v_4 = cursor;
        do {
          // (, line 46
          if (!(in_grouping(g_v, 97, 249))) {
            goto lab5Break;
          }
          // [, line 47
          bra = cursor;
          // or, line 47
          do {
            v_5 = cursor;
            do {
              // (, line 47
              // literal, line 47
              if (!(eq_s(1, L"u"))) {
                goto lab7Break;
              }
              // ], line 47
              ket = cursor;
              if (!(in_grouping(g_v, 97, 249))) {
                goto lab7Break;
              }
              // <-, line 47
              slice_from(L"U");
              goto lab6Break;
            } while (false);
          lab7Continue:;
          lab7Break:
            cursor = v_5;
            // (, line 48
            // literal, line 48
            if (!(eq_s(1, L"i"))) {
              goto lab5Break;
            }
            // ], line 48
            ket = cursor;
            if (!(in_grouping(g_v, 97, 249))) {
              goto lab5Break;
            }
            // <-, line 48
            slice_from(L"I");
          } while (false);
        lab6Continue:;
        lab6Break:
          cursor = v_4;
          goto golab4Break;
        } while (false);
      lab5Continue:;
      lab5Break:
        cursor = v_4;
        if (cursor >= limit) {
          goto lab3Break;
        }
        cursor++;
      golab4Continue:;
      }
    golab4Break:
      goto replab2Continue;
    } while (false);
  lab3Continue:;
  lab3Break:
    cursor = v_3;
    goto replab2Break;
  replab2Continue:;
  }
replab2Break:
  return true;
}

bool ItalianStemmer::r_mark_regions()
{
  int v_1;
  int v_2;
  int v_3;
  int v_6;
  int v_8;
  // (, line 52
  I_pV = limit;
  I_p1 = limit;
  I_p2 = limit;
  // do, line 58
  v_1 = cursor;
  do {
    // (, line 58
    // or, line 60
    do {
      v_2 = cursor;
      do {
        // (, line 59
        if (!(in_grouping(g_v, 97, 249))) {
          goto lab2Break;
        }
        // or, line 59
        do {
          v_3 = cursor;
          do {
            // (, line 59
            if (!(out_grouping(g_v, 97, 249))) {
              goto lab4Break;
            }
            // gopast, line 59
            while (true) {
              do {
                if (!(in_grouping(g_v, 97, 249))) {
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
          // (, line 59
          if (!(in_grouping(g_v, 97, 249))) {
            goto lab2Break;
          }
          // gopast, line 59
          while (true) {
            do {
              if (!(out_grouping(g_v, 97, 249))) {
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
      // (, line 61
      if (!(out_grouping(g_v, 97, 249))) {
        goto lab0Break;
      }
      // or, line 61
      do {
        v_6 = cursor;
        do {
          // (, line 61
          if (!(out_grouping(g_v, 97, 249))) {
            goto lab10Break;
          }
          // gopast, line 61
          while (true) {
            do {
              if (!(in_grouping(g_v, 97, 249))) {
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
        // (, line 61
        if (!(in_grouping(g_v, 97, 249))) {
          goto lab0Break;
        }
        // next, line 61
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
    // setmark pV, line 62
    I_pV = cursor;
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 64
  v_8 = cursor;
  do {
    // (, line 64
    // gopast, line 65
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 249))) {
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
    // gopast, line 65
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 249))) {
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
    // setmark p1, line 65
    I_p1 = cursor;
    // gopast, line 66
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 249))) {
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
    // gopast, line 66
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 249))) {
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
    // setmark p2, line 66
    I_p2 = cursor;
  } while (false);
lab13Continue:;
lab13Break:
  cursor = v_8;
  return true;
}

bool ItalianStemmer::r_postlude()
{
  int among_var;
  int v_1;
  // repeat, line 70
  while (true) {
    v_1 = cursor;
    do {
      // (, line 70
      // [, line 72
      bra = cursor;
      // substring, line 72
      among_var = find_among(a_1, 3);
      if (among_var == 0) {
        goto lab1Break;
      }
      // ], line 72
      ket = cursor;
      switch (among_var) {
      case 0:
        goto lab1Break;
      case 1:
        // (, line 73
        // <-, line 73
        slice_from(L"i");
        break;
      case 2:
        // (, line 74
        // <-, line 74
        slice_from(L"u");
        break;
      case 3:
        // (, line 75
        // next, line 75
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

bool ItalianStemmer::r_RV()
{
  if (!(I_pV <= cursor)) {
    return false;
  }
  return true;
}

bool ItalianStemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool ItalianStemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool ItalianStemmer::r_attached_pronoun()
{
  int among_var;
  // (, line 86
  // [, line 87
  ket = cursor;
  // substring, line 87
  if (find_among_b(a_2, 37) == 0) {
    return false;
  }
  // ], line 87
  bra = cursor;
  // among, line 97
  among_var = find_among_b(a_3, 5);
  if (among_var == 0) {
    return false;
  }
  // (, line 97
  // call RV, line 97
  if (!r_RV()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 98
    // delete, line 98
    slice_del();
    break;
  case 2:
    // (, line 99
    // <-, line 99
    slice_from(L"e");
    break;
  }
  return true;
}

bool ItalianStemmer::r_standard_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  // (, line 103
  // [, line 104
  ket = cursor;
  // substring, line 104
  among_var = find_among_b(a_6, 51);
  if (among_var == 0) {
    return false;
  }
  // ], line 104
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 111
    // call R2, line 111
    if (!r_R2()) {
      return false;
    }
    // delete, line 111
    slice_del();
    break;
  case 2:
    // (, line 113
    // call R2, line 113
    if (!r_R2()) {
      return false;
    }
    // delete, line 113
    slice_del();
    // try, line 114
    v_1 = limit - cursor;
    do {
      // (, line 114
      // [, line 114
      ket = cursor;
      // literal, line 114
      if (!(eq_s_b(2, L"ic"))) {
        cursor = limit - v_1;
        goto lab0Break;
      }
      // ], line 114
      bra = cursor;
      // call R2, line 114
      if (!r_R2()) {
        cursor = limit - v_1;
        goto lab0Break;
      }
      // delete, line 114
      slice_del();
    } while (false);
  lab0Continue:;
  lab0Break:
    break;
  case 3:
    // (, line 117
    // call R2, line 117
    if (!r_R2()) {
      return false;
    }
    // <-, line 117
    slice_from(L"log");
    break;
  case 4:
    // (, line 119
    // call R2, line 119
    if (!r_R2()) {
      return false;
    }
    // <-, line 119
    slice_from(L"u");
    break;
  case 5:
    // (, line 121
    // call R2, line 121
    if (!r_R2()) {
      return false;
    }
    // <-, line 121
    slice_from(L"ente");
    break;
  case 6:
    // (, line 123
    // call RV, line 123
    if (!r_RV()) {
      return false;
    }
    // delete, line 123
    slice_del();
    break;
  case 7:
    // (, line 124
    // call R1, line 125
    if (!r_R1()) {
      return false;
    }
    // delete, line 125
    slice_del();
    // try, line 126
    v_2 = limit - cursor;
    do {
      // (, line 126
      // [, line 127
      ket = cursor;
      // substring, line 127
      among_var = find_among_b(a_4, 4);
      if (among_var == 0) {
        cursor = limit - v_2;
        goto lab1Break;
      }
      // ], line 127
      bra = cursor;
      // call R2, line 127
      if (!r_R2()) {
        cursor = limit - v_2;
        goto lab1Break;
      }
      // delete, line 127
      slice_del();
      switch (among_var) {
      case 0:
        cursor = limit - v_2;
        goto lab1Break;
      case 1:
        // (, line 128
        // [, line 128
        ket = cursor;
        // literal, line 128
        if (!(eq_s_b(2, L"at"))) {
          cursor = limit - v_2;
          goto lab1Break;
        }
        // ], line 128
        bra = cursor;
        // call R2, line 128
        if (!r_R2()) {
          cursor = limit - v_2;
          goto lab1Break;
        }
        // delete, line 128
        slice_del();
        break;
      }
    } while (false);
  lab1Continue:;
  lab1Break:
    break;
  case 8:
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
      among_var = find_among_b(a_5, 3);
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
        // (, line 137
        // call R2, line 137
        if (!r_R2()) {
          cursor = limit - v_3;
          goto lab2Break;
        }
        // delete, line 137
        slice_del();
        break;
      }
    } while (false);
  lab2Continue:;
  lab2Break:
    break;
  case 9:
    // (, line 141
    // call R2, line 142
    if (!r_R2()) {
      return false;
    }
    // delete, line 142
    slice_del();
    // try, line 143
    v_4 = limit - cursor;
    do {
      // (, line 143
      // [, line 143
      ket = cursor;
      // literal, line 143
      if (!(eq_s_b(2, L"at"))) {
        cursor = limit - v_4;
        goto lab3Break;
      }
      // ], line 143
      bra = cursor;
      // call R2, line 143
      if (!r_R2()) {
        cursor = limit - v_4;
        goto lab3Break;
      }
      // delete, line 143
      slice_del();
      // [, line 143
      ket = cursor;
      // literal, line 143
      if (!(eq_s_b(2, L"ic"))) {
        cursor = limit - v_4;
        goto lab3Break;
      }
      // ], line 143
      bra = cursor;
      // call R2, line 143
      if (!r_R2()) {
        cursor = limit - v_4;
        goto lab3Break;
      }
      // delete, line 143
      slice_del();
    } while (false);
  lab3Continue:;
  lab3Break:
    break;
  }
  return true;
}

bool ItalianStemmer::r_verb_suffix()
{
  int among_var;
  int v_1;
  int v_2;
  // setlimit, line 148
  v_1 = limit - cursor;
  // tomark, line 148
  if (cursor < I_pV) {
    return false;
  }
  cursor = I_pV;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 148
  // [, line 149
  ket = cursor;
  // substring, line 149
  among_var = find_among_b(a_7, 87);
  if (among_var == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 149
  bra = cursor;
  switch (among_var) {
  case 0:
    limit_backward = v_2;
    return false;
  case 1:
    // (, line 163
    // delete, line 163
    slice_del();
    break;
  }
  limit_backward = v_2;
  return true;
}

bool ItalianStemmer::r_vowel_suffix()
{
  int v_1;
  int v_2;
  // (, line 170
  // try, line 171
  v_1 = limit - cursor;
  do {
    // (, line 171
    // [, line 172
    ket = cursor;
    if (!(in_grouping_b(g_AEIO, 97, 242))) {
      cursor = limit - v_1;
      goto lab0Break;
    }
    // ], line 172
    bra = cursor;
    // call RV, line 172
    if (!r_RV()) {
      cursor = limit - v_1;
      goto lab0Break;
    }
    // delete, line 172
    slice_del();
    // [, line 173
    ket = cursor;
    // literal, line 173
    if (!(eq_s_b(1, L"i"))) {
      cursor = limit - v_1;
      goto lab0Break;
    }
    // ], line 173
    bra = cursor;
    // call RV, line 173
    if (!r_RV()) {
      cursor = limit - v_1;
      goto lab0Break;
    }
    // delete, line 173
    slice_del();
  } while (false);
lab0Continue:;
lab0Break:
  // try, line 175
  v_2 = limit - cursor;
  do {
    // (, line 175
    // [, line 176
    ket = cursor;
    // literal, line 176
    if (!(eq_s_b(1, L"h"))) {
      cursor = limit - v_2;
      goto lab1Break;
    }
    // ], line 176
    bra = cursor;
    if (!(in_grouping_b(g_CG, 99, 103))) {
      cursor = limit - v_2;
      goto lab1Break;
    }
    // call RV, line 176
    if (!r_RV()) {
      cursor = limit - v_2;
      goto lab1Break;
    }
    // delete, line 176
    slice_del();
  } while (false);
lab1Continue:;
lab1Break:
  return true;
}

bool ItalianStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  // (, line 181
  // do, line 182
  v_1 = cursor;
  do {
    // call prelude, line 182
    if (!r_prelude()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 183
  v_2 = cursor;
  do {
    // call mark_regions, line 183
    if (!r_mark_regions()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = v_2;
  // backwards, line 184
  limit_backward = cursor;
  cursor = limit;
  // (, line 184
  // do, line 185
  v_3 = limit - cursor;
  do {
    // call attached_pronoun, line 185
    if (!r_attached_pronoun()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  // do, line 186
  v_4 = limit - cursor;
  do {
    // (, line 186
    // or, line 186
    do {
      v_5 = limit - cursor;
      do {
        // call standard_suffix, line 186
        if (!r_standard_suffix()) {
          goto lab5Break;
        }
        goto lab4Break;
      } while (false);
    lab5Continue:;
    lab5Break:
      cursor = limit - v_5;
      // call verb_suffix, line 186
      if (!r_verb_suffix()) {
        goto lab3Break;
      }
    } while (false);
  lab4Continue:;
  lab4Break:;
  } while (false);
lab3Continue:;
lab3Break:
  cursor = limit - v_4;
  // do, line 187
  v_6 = limit - cursor;
  do {
    // call vowel_suffix, line 187
    if (!r_vowel_suffix()) {
      goto lab6Break;
    }
  } while (false);
lab6Continue:;
lab6Break:
  cursor = limit - v_6;
  cursor = limit_backward; // do, line 189
  v_7 = cursor;
  do {
    // call postlude, line 189
    if (!r_postlude()) {
      goto lab7Break;
    }
  } while (false);
lab7Continue:;
lab7Break:
  cursor = v_7;
  return true;
}

bool ItalianStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<ItalianStemmer>(o) != nullptr;
}

int ItalianStemmer::hashCode()
{
  return ItalianStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext