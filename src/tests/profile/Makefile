include ../Makefile.common

LIBS = ../../libmm3d/libmm3d.a

# Add profile tests here

PROFILES = \
		  load_md3.prof \
		  calc_fnormals.prof \
		  calc_normals.prof \

all: $(PROFILES)

clean:
	rm -rf *.prof *.moc core core.*

.cc.prof:
	g++ $(CXXFLAGS) $(LFLAGS) -lGL -lGLU -pg -o $*.prof $< $(LIBS)

