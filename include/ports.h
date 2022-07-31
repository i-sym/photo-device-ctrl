#ifndef PORTS_H
#define PORTS_H

#include <Arduino.h>
void open(int port);
void close(int port);
void testPorts();
void setupPorts();

#endif // PORTS_H