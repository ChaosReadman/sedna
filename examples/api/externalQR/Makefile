
CXX ?= g++

CXXFLAGS += -shared -fPIC -Wall -std=c++11 -Wall -I/usr/local/sedna/include $(shell pkg-config --cflags opencv4)
LDFLAGS += $(shell pkg-config --libs --static opencv4)

#CXXFLAGS = -shared -fPIC -std=c++11 -I../../../../include -I/usr/local/sedna/include `pkg-config --cflags --libs opencv4`

SO_EXT=.so
qr$(SO_EXT): qr.cpp
	g++ base64.cpp QrCode.cpp qr.cpp $(CXXFLAGS) $(LDFLAGS) -oqr$(SO_EXT)

LIB = qrcodegen
LIBFILE = lib$(LIB).a
LIBOBJ = QrCode.o

%: %.o $(LIBFILE)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< -L . -l $(LIB)


# The library
$(LIBFILE): $(LIBOBJ)
	$(AR) -crs $@ -- $^

# Object files
%.o: %.cpp .deps/timestamp
	$(CXX) $(CXXFLAGS) -c -o $@ -MMD -MF .deps/$*.d $<


clean:
	-rm -rf *.o *.obj *.lib *.exp *$(SO_EXT) $(LIBOBJ) $(LIBFILE)
