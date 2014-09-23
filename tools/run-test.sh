#!/bin/bash

search () {
   for dir in *
   do
      if [ -f "$dir" ] ; then   # ==> Если это file (-f)...
          exec nmap2json.pl -f "${dir}"
      fi
   done
}

# - Main -
if [ $# = 0 ] ; then
   cd `pwd`    # ==> Если аргумент командной строки отсутствует, то используется текущий каталог.
else
   cd $1       # ==> иначе перейти в заданный каталог.
fi
echo "Начальный каталог = `pwd`"

   search   # ==> Вызвать функцию поиска.

exit 0
