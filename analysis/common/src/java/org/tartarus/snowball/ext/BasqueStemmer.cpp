using namespace std;

#include "BasqueStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    BasqueStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    BasqueStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"idea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bidea", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kidea", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pidea", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kundea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"galea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tailea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzailea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gunea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kunea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzaga", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gaia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aldia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"taldia", 12, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"karia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"garria", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"karria", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ka", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzaka", 17, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"la", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mena", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pena", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kina", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ezina", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tezina", 23, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kuna", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tuna", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kizuna", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"era", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bera", 28, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arabera", 29, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kera", 28, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pera", 28, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"orra", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"korra", 33, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dura", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gura", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kura", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tura", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eta", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"keta", 39, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gailua", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eza", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erreza", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tza", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gaitza", 44, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kaitza", 44, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kuntza", 44, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ide", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bide", 48, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kide", 48, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pide", 48, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kunde", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzake", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzeke", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"le", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gale", 55, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"taile", 55, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzaile", 55, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gune", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kune", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tze", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atze", 61, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gai", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aldi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"taldi", 64, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ki", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ari", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kari", 67, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lari", 67, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tari", 67, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"etari", 70, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"garri", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"karri", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arazi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tarazi", 74, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"an", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ean", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"rean", 77, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kan", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"etan", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atseden", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"men", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pen", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kin", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"rekin", 84, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ezin", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tezin", 86, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tun", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kizun", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"go", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ago", 90, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tio", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dako", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"or", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kor", 94, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzat", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"du", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gailu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atu", 99, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aldatu", 100, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tatu", 100, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"baditu", 99, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ez", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"errez", 104, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzez", 104, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gaitz", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kaitz", -1, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    BasqueStemmer::a_1 = {
        make_shared<org::tartarus::snowball::Among>(L"ada", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kada", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"anda", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"denda", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gabea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kabea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aldea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kaldea", 6, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"taldea", 6, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ordea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zalea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzalea", 10, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gilea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"emea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kumea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enea", 15, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zionea", 15, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"unea", 15, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gunea", 18, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aurrea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kotea", 22, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"artea", 22, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ostea", 22, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"etxea", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ga", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"anga", 27, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gaia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aldia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"taldia", 30, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"handia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mendia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"geia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"egia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"degia", 35, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tegia", 35, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nahia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ohia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tokia", 40, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"koia", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aria", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"karia", 44, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"laria", 44, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"taria", 44, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eria", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"keria", 48, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"teria", 48, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"garria", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"larria", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kirria", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"duria", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ezia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bizia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ontzia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ka", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"joka", 60, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aurka", 60, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ska", 60, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"xka", 60, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zka", 60, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gibela", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gela", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kaila", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"skila", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tila", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ola", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"na", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kana", 72, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ena", 72, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"garrena", 74, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gerrena", 74, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"urrena", 74, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zaina", 72, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzaina", 78, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kina", 72, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mina", 72, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"garna", 72, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"una", 72, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"duna", 83, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asuna", 83, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tasuna", 85, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ondoa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kondoa", 87, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ngoa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zioa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"koa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"takoa", 91, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zkoa", 91, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"noa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zinoa", 94, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aroa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"taroa", 96, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zaroa", 96, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eroa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oroa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"osoa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"toa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ttoa", 102, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ztoa", 102, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"txoa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzoa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F1oa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ra", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ara", 108, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dara", 109, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"liara", 109, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tiara", 109, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tara", 109, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"etara", 113, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzara", 109, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bera", 108, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kera", 108, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pera", 108, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ora", 108, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzarra", 108, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"korra", 108, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tra", 108, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"osa", 123, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ta", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eta", 125, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"keta", 126, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sta", 125, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dua", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mendua", 129, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ordua", 129, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lekua", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"burua", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"durua", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tsua", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tua", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mentua", 136, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"estua", 136, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"txua", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zua", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzua", 140, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"za", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eza", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eroza", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tza", 142, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"koitza", 145, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"antza", 145, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gintza", 145, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kintza", 145, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kuntza", 145, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gabe", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kabe", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kide", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alde", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kalde", 154, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"talde", 154, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"orde", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ge", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zale", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzale", 159, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gile", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eme", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kume", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ne", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zione", 164, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"une", 164, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gune", 166, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"pe", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aurre", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"te", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kote", 170, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arte", 170, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oste", 170, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"etxe", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gai", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"di", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aldi", 176, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"taldi", 177, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"geldi", 176, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"handi", 176, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mendi", 176, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gei", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"egi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"degi", 183, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tegi", 183, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nahi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ohi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ki", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"toki", 188, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"goi", 190, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"koi", 190, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ari", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kari", 193, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lari", 193, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tari", 193, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"garri", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"larri", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kirri", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"duri", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ti", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ontzi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F1i", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ak", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ek", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tarik", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gibel", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ail", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kail", 209, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kan", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tan", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"etan", 212, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"en", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ren", 214, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"garren", 215, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gerren", 215, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"urren", 215, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ten", 214, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzen", 214, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zain", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzain", 221, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kin", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"min", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dun", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asun", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tasun", 226, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aizun", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ondo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kondo", 229, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"go", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ngo", 231, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zio", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ko", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"trako", 234, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tako", 234, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"etako", 236, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eko", 234, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tariko", 234, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sko", 234, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tuko", 234, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"minutuko", 241, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zko", 234, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"no", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zino", 244, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ro", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aro", 246, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"igaro", 247, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"taro", 247, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zaro", 247, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ero", 246, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"giro", 246, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oro", 246, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oso", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"to", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tto", 255, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zto", 255, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"txo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gintzo", 259, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F1o", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zp", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dar", 263, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"behar", 263, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zehar", 263, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"liar", 263, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tiar", 263, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tar", 263, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzar", 263, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"or", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kor", 271, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"os", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ket", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"du", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mendu", 275, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ordu", 275, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"leku", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"buru", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"duru", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tsu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tatu", 282, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"mentu", 282, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"estu", 282, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"txu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzu", 287, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gintzu", 288, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"z", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ez", 290, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eroz", 290, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tz", 290, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"koitz", 293, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    BasqueStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"zlea", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"keria", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"la", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"era", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dade", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tade", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"date", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tate", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"gi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ki", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ik", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lanik", 10, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"rik", 10, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"larik", 12, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ztik", 10, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"go", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ro", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ero", 16, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"to", -1, 1, L"",
                                                    methodObject)};
std::deque<wchar_t> const BasqueStemmer::g_v = {17, 65, 16};

void BasqueStemmer::copy_from(shared_ptr<BasqueStemmer> other)
{
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  I_pV = other->I_pV;
  SnowballProgram::copy_from(other);
}

bool BasqueStemmer::r_mark_regions()
{
  int v_1;
  int v_2;
  int v_3;
  int v_6;
  int v_8;
  // (, line 25
  I_pV = limit;
  I_p1 = limit;
  I_p2 = limit;
  // do, line 31
  v_1 = cursor;
  do {
    // (, line 31
    // or, line 33
    do {
      v_2 = cursor;
      do {
        // (, line 32
        if (!(in_grouping(g_v, 97, 117))) {
          goto lab2Break;
        }
        // or, line 32
        do {
          v_3 = cursor;
          do {
            // (, line 32
            if (!(out_grouping(g_v, 97, 117))) {
              goto lab4Break;
            }
            // gopast, line 32
            while (true) {
              do {
                if (!(in_grouping(g_v, 97, 117))) {
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
          // (, line 32
          if (!(in_grouping(g_v, 97, 117))) {
            goto lab2Break;
          }
          // gopast, line 32
          while (true) {
            do {
              if (!(out_grouping(g_v, 97, 117))) {
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
      // (, line 34
      if (!(out_grouping(g_v, 97, 117))) {
        goto lab0Break;
      }
      // or, line 34
      do {
        v_6 = cursor;
        do {
          // (, line 34
          if (!(out_grouping(g_v, 97, 117))) {
            goto lab10Break;
          }
          // gopast, line 34
          while (true) {
            do {
              if (!(in_grouping(g_v, 97, 117))) {
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
        // (, line 34
        if (!(in_grouping(g_v, 97, 117))) {
          goto lab0Break;
        }
        // next, line 34
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
    // setmark pV, line 35
    I_pV = cursor;
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // do, line 37
  v_8 = cursor;
  do {
    // (, line 37
    // gopast, line 38
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 117))) {
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
    // gopast, line 38
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 117))) {
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
    // setmark p1, line 38
    I_p1 = cursor;
    // gopast, line 39
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 117))) {
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
    // gopast, line 39
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 117))) {
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
    // setmark p2, line 39
    I_p2 = cursor;
  } while (false);
lab13Continue:;
lab13Break:
  cursor = v_8;
  return true;
}

bool BasqueStemmer::r_RV()
{
  if (!(I_pV <= cursor)) {
    return false;
  }
  return true;
}

bool BasqueStemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool BasqueStemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool BasqueStemmer::r_aditzak()
{
  int among_var;
  // (, line 49
  // [, line 50
  ket = cursor;
  // substring, line 50
  among_var = find_among_b(a_0, 109);
  if (among_var == 0) {
    return false;
  }
  // ], line 50
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 61
    // call RV, line 61
    if (!r_RV()) {
      return false;
    }
    // delete, line 61
    slice_del();
    break;
  case 2:
    // (, line 63
    // call R2, line 63
    if (!r_R2()) {
      return false;
    }
    // delete, line 63
    slice_del();
    break;
  case 3:
    // (, line 65
    // <-, line 65
    slice_from(L"atseden");
    break;
  case 4:
    // (, line 67
    // <-, line 67
    slice_from(L"arabera");
    break;
  case 5:
    // (, line 69
    // <-, line 69
    slice_from(L"baditu");
    break;
  }
  return true;
}

bool BasqueStemmer::r_izenak()
{
  int among_var;
  // (, line 74
  // [, line 75
  ket = cursor;
  // substring, line 75
  among_var = find_among_b(a_1, 295);
  if (among_var == 0) {
    return false;
  }
  // ], line 75
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 105
    // call RV, line 105
    if (!r_RV()) {
      return false;
    }
    // delete, line 105
    slice_del();
    break;
  case 2:
    // (, line 107
    // call R2, line 107
    if (!r_R2()) {
      return false;
    }
    // delete, line 107
    slice_del();
    break;
  case 3:
    // (, line 109
    // <-, line 109
    slice_from(L"jok");
    break;
  case 4:
    // (, line 111
    // call R1, line 111
    if (!r_R1()) {
      return false;
    }
    // delete, line 111
    slice_del();
    break;
  case 5:
    // (, line 113
    // <-, line 113
    slice_from(L"tra");
    break;
  case 6:
    // (, line 115
    // <-, line 115
    slice_from(L"minutu");
    break;
  case 7:
    // (, line 117
    // <-, line 117
    slice_from(L"zehar");
    break;
  case 8:
    // (, line 119
    // <-, line 119
    slice_from(L"geldi");
    break;
  case 9:
    // (, line 121
    // <-, line 121
    slice_from(L"igaro");
    break;
  case 10:
    // (, line 123
    // <-, line 123
    slice_from(L"aurka");
    break;
  }
  return true;
}

bool BasqueStemmer::r_adjetiboak()
{
  int among_var;
  // (, line 127
  // [, line 128
  ket = cursor;
  // substring, line 128
  among_var = find_among_b(a_2, 19);
  if (among_var == 0) {
    return false;
  }
  // ], line 128
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 131
    // call RV, line 131
    if (!r_RV()) {
      return false;
    }
    // delete, line 131
    slice_del();
    break;
  case 2:
    // (, line 133
    // <-, line 133
    slice_from(L"z");
    break;
  }
  return true;
}

bool BasqueStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  // (, line 139
  // do, line 140
  v_1 = cursor;
  do {
    // call mark_regions, line 140
    if (!r_mark_regions()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // backwards, line 141
  limit_backward = cursor;
  cursor = limit;
  // (, line 141
  // repeat, line 142
  while (true) {
    v_2 = limit - cursor;
    do {
      // call aditzak, line 142
      if (!r_aditzak()) {
        goto lab2Break;
      }
      goto replab1Continue;
    } while (false);
  lab2Continue:;
  lab2Break:
    cursor = limit - v_2;
    goto replab1Break;
  replab1Continue:;
  }
replab1Break:
  // repeat, line 143
  while (true) {
    v_3 = limit - cursor;
    do {
      // call izenak, line 143
      if (!r_izenak()) {
        goto lab4Break;
      }
      goto replab3Continue;
    } while (false);
  lab4Continue:;
  lab4Break:
    cursor = limit - v_3;
    goto replab3Break;
  replab3Continue:;
  }
replab3Break:
  // do, line 144
  v_4 = limit - cursor;
  do {
    // call adjetiboak, line 144
    if (!r_adjetiboak()) {
      goto lab5Break;
    }
  } while (false);
lab5Continue:;
lab5Break:
  cursor = limit - v_4;
  cursor = limit_backward;
  return true;
}

bool BasqueStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<BasqueStemmer>(o) != nullptr;
}

int BasqueStemmer::hashCode()
{
  return BasqueStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext