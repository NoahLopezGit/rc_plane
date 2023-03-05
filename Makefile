my_basic_comms : my_basic_comms.o
        g++ my_basic_comms.o -o my_basic_comms -L. -lrf24

my_basic_comms.o : my_basic_comms.cpp
        g++ -c my_basic_comms.cpp -o my_basic_comms.o

clean:
        rm -f my_basic_comms.o
