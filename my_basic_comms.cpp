/*
 * See documentation at https://nRF24.github.io/RF24
 * See License information at root directory of this library
 * Author: Brendan Doherty (2bndy5)
 */

/**
 * A simple example of sending data from 1 nRF24L01 transceiver to another.
 *
 * This example was written to be used on 2 devices acting as "nodes".
 * Use `ctrl+c` to quit at any time.
 */
#include <ctime>       // time()
#include <iostream>    // cin, cout, endl
#include <string>      // string, getline()
#include <time.h>      // CLOCK_MONOTONIC_RAW, timespec, clock_gettime()
#include <RF24/RF24.h> // RF24, RF24_PA_LOW, delay()
#include <fstream>
using namespace std;

#define Debug_mode false
/****************** Linux ***********************/
// Radio CE Pin, CSN Pin, SPI Speed
// CE Pin uses GPIO number with BCM and SPIDEV drivers, other platforms use their own pin numbering
// CS Pin addresses the SPI bus number at /dev/spidev<a>.<b>
// ie: RF24 radio(<ce_pin>, <a>*10+<b>); spidev1.0 is 10, spidev1.1 is 11 etc..

// Generic:
RF24 radio(485, 0);
/****************** Linux (BBB,x86,etc) ***********************/
// See http://nRF24.github.io/RF24/pages.html for more information on usage
// See http://iotdk.intel.com/docs/master/mraa/ for more information on MRAA
// See https://www.kernel.org/doc/Documentation/spi/spidev for more information on SPIDEV

// For this example, we'll be using a payload containing
// a single float number that will be incremented
// on every successful transmission
struct MyData {
	uint8_t test[32];
};
MyData payload;


void slave();   // prototype of the RX node's behavior

// custom defined timer for evaluating transmission time in microseconds
struct timespec startTimer, endTimer;
uint32_t getMicros(); // prototype to get ellapsed time in microseconds

int main(int argc, char** argv)
{

    // perform hardware check
    if (!radio.begin()) {
        cout << "radio hardware is not responding!!" << endl;
        return 0; // quit now
    }

    // print example's name
    cout << argv[0] << endl;

    // Let these addresses be used for the pair
    const uint64_t pipeIn = 0x662266;
    // It is very helpful to think of an address as a path instead of as
    // an identifying device destination

    // save on transmission time by setting the radio to only transmit the
    // number of bytes we need to transmit a float
    radio.setPayloadSize(sizeof(payload)); // float datatype occupies 4 bytes

    // Set the PA Level low to try preventing power supply related problems
    // because these examples are likely run with nodes in close proximity to
    // each other.
    radio.setPALevel(RF24_PA_MIN); // RF24_PA_MAX is default.

    // set the TX address of the RX node into the TX pipe
    radio.openWritingPipe(pipeIn); // always uses pipe 0

    // set the RX address of the TX node into a RX pipe
    radio.openReadingPipe(1, pipeIn); // using pipe 1

    // my setup
    radio.setAutoAck(true);
    radio.setDataRate(RF24_2MBPS);
    radio.setChannel(108);
    radio.enableDynamicPayloads();
    radio.setCRCLength(RF24_CRC_16);
    radio.setRetries(10, 15);
    // For debugging info
    // radio.printDetails();       // (smaller) function that prints raw register values
    // radio.printPrettyDetails(); // (larger) function that prints human readable data

    // ready to execute program now
    slave(); // calls master() or slave() based on user input
    return 0;
}

/**
 * make this node act as the receiver
 */
void slave()
{
    ofstream myfile;
    myfile.open ("output.txt");
    radio.startListening(); // put radio in RX mode
    uint32_t bit_cnt=0;
//    time_t startTimer = time(nullptr);       // start a timer
    while (bit_cnt < 32150) { // use 6 second timeout
        uint8_t pipe;
        if (radio.available(&pipe)) {                        // is there a payload? get the pipe number that recieved it
            uint8_t bytes = radio.getPayloadSize();          // get the size of the payload
            radio.read(&payload, bytes);                     // fetch payload from FIFO
            //cout << "Received " << (unsigned int)bytes;      // print the size of the payload
            //cout << " bytes on pipe " << (unsigned int)pipe; // print the pipe number
	    //cout << ": ";
            //cout << (uint32_t)payload;
	    //cout << endl;                 // print the payload's value
            for (int itr=0;itr<32;itr++){
            	myfile << (int)payload.test[itr];
	    };
            //startTimer = time(nullptr);                      // reset timer
	    bit_cnt++;
        }
    }
    cout << "1 MB transfered" << endl;
    myfile.close();
    radio.stopListening();
}

/**
 * Calculate the ellapsed time in microseconds
 */
uint32_t getMicros()
{
    // this function assumes that the timer was started using
    // `clock_gettime(CLOCK_MONOTONIC_RAW, &startTimer);`

    clock_gettime(CLOCK_MONOTONIC_RAW, &endTimer);
    uint32_t seconds = endTimer.tv_sec - startTimer.tv_sec;
    uint32_t useconds = (endTimer.tv_nsec - startTimer.tv_nsec) / 1000;

    return ((seconds)*1000 + useconds) + 0.5;
}
