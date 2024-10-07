INPUT = 1
OUTPUT = 1out
build-sharpen: ./a.out

./a.out: image_sharpener.cpp libppm.cpp
	g++ -g image_sharpener.cpp libppm.cpp

run-sharpen: ./a.out
	./a.out ../images/$(INPUT).ppm ../images/$(OUTPUT).ppm

clean:
	rm ./a.out



build-pipe: ./b.out

./b.out: pipe.cpp libppm.cpp
	g++ -g pipe.cpp libppm.cpp

run-pipe: ./b.out
	./a.out ../images/$(INPUT).ppm ../images/1pipe.ppm





build-sync: ./c.out

./c.out: synchonization.cpp libppm.cpp
	g++ -g synchronization.cpp libppm.cpp

run-sync: ./c.out
	./a.out ../images/$(INPUT).ppm ../images/1sync.ppm