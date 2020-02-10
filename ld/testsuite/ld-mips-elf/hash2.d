#source: start.s
#readelf: -d -s -D
#ld: -shared --hash-style=gnu
#target: [check_shared_lib_support] 
#xfail: mips*-*-irix*

#...
 +0x[0-9a-z]+ +\(MIPS_XHASH\) +0x[0-9a-z]+
#...
 +[0-9]+ +[0-9]+: +[0-9a-f]+ +[0-9]+ +NOTYPE +GLOBAL +DEFAULT +[1-9] _start
#...
 +[0-9]+ +[0-9]+: +[0-9a-f]+ +[0-9]+ +NOTYPE +GLOBAL +DEFAULT +[1-9] main
#...
 +[0-9]+ +[0-9]+: +[0-9a-f]+ +[0-9]+ +NOTYPE +GLOBAL +DEFAULT +[1-9] start
#...
 +[0-9]+ +[0-9]+: +[0-9a-f]+ +[0-9]+ +NOTYPE +GLOBAL +DEFAULT +[1-9] __start
#...
