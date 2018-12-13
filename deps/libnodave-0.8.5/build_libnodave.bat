move win\libnodave.dll win\libnodave.dll.bak
move win\libnodave.lib win\libnodave.lib.bak

cl -I"$(VCPATH)\include" -I"$(SDKPATH)\include" -c -DBCCWIN -DDAVE_LITTLE_ENDIAN -TC -DDOEXPORT nodave.c
cl -I"$(VCPATH)\include" -I"$(SDKPATH)\include" -c -DBCCWIN -DDAVE_LITTLE_ENDIAN -TC -DDOEXPORT setportw.c
cl -I"$(VCPATH)\include" -I"$(SDKPATH)\include" -c -DBCCWIN -DDAVE_LITTLE_ENDIAN -TC -DDOEXPORT openSocketw.c
cl -I"$(VCPATH)\include" -I"$(SDKPATH)\include" -c -DBCCWIN -DDAVE_LITTLE_ENDIAN -TC -DDOEXPORT openS7online

link /LIBPATH:"$(VCPATH)\lib" /DEF:libnodave.DEF /DLL nodave.obj setportw.obj openSocketw.obj openS7online.obj ws2_32.lib /OUT:libnodave.dll

move libnodave.dll win\libnodave.dll
move libnodave.lib win\libnodave.lib