
#include "ports.h"
#include "error.h"

int mapPort(int port)
{
  return port + 21;
}

void open(int port)
{
  digitalWrite(mapPort(port), LOW);
}
void close(int port)
{
  digitalWrite(mapPort(port), HIGH);
}
void testPorts()
{
  for (int i = 0; i < 16; i++)
  {
    open(i);
    delay(100);
    close(i);
  }
}
void setupPorts(){
    for (int i = 1; i < 17; i++)
  {
    pinMode(mapPort(i), OUTPUT);
    close(i);
  }
}