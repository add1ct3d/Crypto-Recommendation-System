CC=g++
CFLAGS=-std=c++11	-g3	
.PHONY:clean

all: recommend


recommend:	cluster_init.o	main.o cluster_assign.o	cluster_update.o	utility.o	euclidean.o	cosine.o lsh_euclidean.o lsh_cosine.o recommend.o
	@echo	"\tCompile	Recommendation System";
	$(CC)	$(CFLAGS)	main.o cluster_init.o	cluster_assign.o	cluster_update.o	utility.o	lsh_euclidean.o lsh_cosine.o recommend.o	euclidean.o	cosine.o	-o	recommend;


main.o :
	$(CC)	$(CFLAGS)	-c main.cpp	


recommend.o: 
	$(CC)	$(CFLAGS)	-c recommend.cpp	

cluster_init.o:
	$(CC)	$(CFLAGS)	-c cluster_init.cpp	

cluster_update.o:	
	$(CC)	$(CFLAGS)	-c cluster_update.cpp

cluster_assign.o:
	$(CC)	$(CFLAGS)	-c cluster_assign.cpp	

utility.o:	
	$(CC)	$(CFLAGS)	-c utility.cpp

cube.o:
	$(CC)	$(CFLAGS)	-c	cube.cpp 

euclidean.o:
	$(CC)	$(CFLAGS)	-c	euclidean.cpp

cosine.o:	
	$(CC)	$(CFLAGS)	-c	cosine.cpp 

lsh_euclidean.o:
	$(CC)	$(CFLAGS)	-c	lsh_euclidean.cpp

lsh_cosine.o:	
	$(CC)	$(CFLAGS)	-c	lsh_cosine.cpp	

clean:
	rm	*.o;