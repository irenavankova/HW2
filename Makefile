
EXECUTABLES = mpi_bug1 mpi_bug2 mpi_bug3 mpi_bug4 mpi_bug5 mpi_bug6 mpi_bug7 ssort
COMPILER = mpicc
FLAGS = -O3 -Wall

all: $(EXECUTABLES)


mpi_bug1: mpi_bug1.c
	$(COMPILER) $(FLAGS) mpi_bug1.c -o mpi_bug1

bug1: mpi_bug1
	mpirun -np 4 ./mpi_bug1

mpi_bug2: mpi_bug2.c
	$(COMPILER) $(FLAGS) mpi_bug2.c -o mpi_bug2

bug2: mpi_bug2
	mpirun -np 4 ./mpi_bug2

mpi_bug3: mpi_bug3.c
	$(COMPILER) $(FLAGS) mpi_bug3.c -o mpi_bug3

bug3: mpi_bug3
	mpirun -np 4 ./mpi_bug3

mpi_bug4: mpi_bug4.c
	$(COMPILER) $(FLAGS) mpi_bug4.c -o mpi_bug4

bug4: mpi_bug4
	mpirun -np 4 ./mpi_bug4

mpi_bug5: mpi_bug5.c
	$(COMPILER) $(FLAGS) mpi_bug5.c -o mpi_bug5

bug5: mpi_bug5
	mpirun -np 4 ./mpi_bug5

mpi_bug6: mpi_bug6.c
	$(COMPILER) $(FLAGS) mpi_bug6.c -o mpi_bug6

bug6: mpi_bug6
	mpirun -np 4 ./mpi_bug6

mpi_bug7: mpi_bug7.c
	$(COMPILER) $(FLAGS) mpi_bug7.c -o mpi_bug7

bug7: mpi_bug7
	mpirun -np 4 ./mpi_bug7

ssort: ssort.c
	$(COMPILER) $(FLAGS) ssort.c -o ssort

sort_all: ssort
	mpirun -np 4 ./ssort

clean:
	rm -rf $(EXECUTABLES)
