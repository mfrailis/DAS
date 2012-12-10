
CXX = g++ -Wall -Wno-unknown-pragmas
CXXFLAGS = -DDATABASE_MYSQL -I/home/frailis/Osservatorio/software/CIWS/odb/build/include
LDFLAGS = -L/home/frailis/Osservatorio/software/CIWS/odb/build/lib -lodb-mysql -lodb -lodb-boost

src = ddl_instance-odb.cxx ddl_instance.cxx  driver.cxx
prog = test_odb

all: $(prog)

$(prog): $(src)
	$(CXX) $(CXXFLAGS) $^ -o $@  $(LDFLAGS)

clean:
	rm *.o $(prog)
