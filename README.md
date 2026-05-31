# Stock Market Simulator C++

Simulation of the stock market by modelling price dynamics via Geometric Brownian Motion and including operations impact on share prices. Organized using OOP and optimized performance through gprof profiling and STL data structures.

Authors: Arturo Baltanas, Marina Maroto and Icíar Arnal

Project done for Avanced Programming course. Project idea by Andrea De Girolamo andrea.de-girolamo@tum.de and Michael Pio Basile michael.basile@tum.de.

### Example of the Simulation for 80 days: 
![Simulation Results](SimulationTest.png)

---
### Build and Run:
Run the following commands from the root directory after cloning the repository:
```bash
mkdir build
cd build
cmake ..
make
```
To execute run
```bash
./TradingApp ../config.json
```
