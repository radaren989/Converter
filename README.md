# Converter
 converts csv to bin and bin to xml and can validate xml with xsd file
 
 compile with gcc converter.c -o converter.out -I/usr/include/libxml2 -lxml2
 
 (libxml2-dev library required)
 
 run with converter.c src.file dest.file n
 
 n = 1 -> csv to bin
 n = 2 -> bin to xml
 n = 3 -> validates xml with given xsd file
