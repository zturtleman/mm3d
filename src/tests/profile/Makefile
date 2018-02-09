include ../Makefile.common

LIBS = ../../libmm3d/libmm3d.a

# Add profile tests here

PROFILES = \
		  load_md3.prof \
		  calc_fnormals.prof \
		  calc_normals.prof \
		  group_triangles.prof \

all: $(PROFILES)

clean:
	rm -rf *.prof *.moc core core.* *.gcno *.gcda

.cc.prof:
	$(CXX) $(CXXFLAGS) -pg -o $*.prof $< $(LIBS) $(TESTLIBS)

