

CF=-I../../../../include -I/usr/local/sedna/include -fPIC

OS := $(shell (uname | sed 's/^CYGWIN_NT.*$$/CYGWIN_NT/'))
ifeq ("$(OS)","CYGWIN_NT")

SO_EXT=.dll

uuid$(SO_EXT): uuid.cpp
	cl /LD $(CF) uuid.cpp

else

SO_EXT=.so

uuid$(SO_EXT): uuid.cpp
	gcc -shared $(CF) uuid.cpp -o uuid$(SO_EXT)

endif


clean:
	-rm -rf *.o *.obj *.lib *.exp *$(SO_EXT)
