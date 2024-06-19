# skiTester
## The purpose is to create ski glide testing equipment. 
Test results are based on the time used between the magnet gates. The idea is not unique and already has several commercial implementations. 
However, the price of these is not in any way justifiable for a hobbyist skier, and a hobbyist's mindset includes doing things myself.

## My implementation differs from other hardware in terms of the user interface. 
My goal is to make an easy-to-adopt user interface based on a web application. At the moment, the terminal device and the microcontroller 
are communicating via wifi connection, because the microcontroller also acts as a web application server.

## In the future
The biggest challenges under the solution are related to the hardware. I'm looking for an (affordable) way to create a strong enough magnetic
field so that the hall sensor connected to the microcontroller can reliably recognize the passage of the gate.

On the software side, in the future I want to switch from a wifi connection to use a bluetooth connection. I would like to use the Web Bluetooth API, 
so that the web application side of the software could be moved to its own server. This reduces the hardware requirements of the microcontroller and 
increases the possibilities for further development of the user interface.
