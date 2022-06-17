
#include <cstdlib>
#include <iostream>
#include <string>
using namespace std;

int main()
{
               // Первая строка -- особый случай.
string last;
               if (!getline(cin, last))
                              return EXIT_FAILURE;          // Не смогли прочитать и одной строки.

               cout << last << endl;          // Вывести первую строку.

               // Продолжить работу с остатком файла.
               for (string next; getline(cin, next);)
               {
                              if (last != next)
                              {
                                             cout << next << endl;
                                             last = next;
                              }
               }
               //asdasdkalskdm
               return EXIT_SUCCESS;
}