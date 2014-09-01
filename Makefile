builder : builder.o finger-extractor.o fft.o util.o wave-processor.o
	g++ -g -std=c++11 -pthread -o builder builder.o finger-extractor.o fft.o util.o wave-processor.o
matcher : matcher.o finger-extractor.o searcher.o fft.o util.o wave-processor.o
	g++ -g -std=c++11 -o matcher matcher.o searcher.o finger-extractor.o fft.o util.o wave-processor.o
matcher.o : matcher.cc util.h
	g++ -g -std=c++11 -c matcher.cc
builder.o : builder.cc util.h
	g++ -g -std=c++11 -c builder.cc
finger-extractor.o : finger-extractor.cc util.h
	g++ -g -std=c++11 -c finger-extractor.cc
fft.o : fft.cc fft.h
	g++ -g -std=c++11 -c fft.cc
searcher.o : searcher.cc searcher.h
	g++ -g -std=c++11 -c searcher.cc
util.o : util.cc util.h
	g++ -g -std=c++11 -c util.cc
wave-processor.o : wave-processor.cc wave-processor.h
	g++ -g -std=c++11 -c wave-processor.cc
clean :
	rm matcher builder matcher.o builder.o finger-extractor.o fft.o searcher.o util.o wave-processor.o
