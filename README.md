# Stock Market Simulator C++

This project's goal is to simulate the performance of certain portfolios in a simulated stock market. The project is divided into two main simulations: the market simulation (Geometric Brownian Motion) and the investor simulation. The final version is shown including OOP and certain optimizations.

Authors: Arturo Baltanas, Marina Maroto and Icíar Arnal

Project done for Avanced Programming course. Project idea by Andrea De Girolamo andrea.de-girolamo@tum.de and Michael Pio Basile michael.basile@tum.de.

---
### Implementation details

The simulation engine is built upon a modular architecture that separates market physics from investor behavior.

#### 1. Market Price Dynamics

The `PriceModel` class calculates the evolution of asset prices by combining stochastic calculus with market impact logic.

**Geometric Brownian Motion (GBM):**  
Price changes are modeled using the standard GBM formula:

$$
S_{t+\Delta t} = S_t \exp\left((\mu - \tfrac{1}{2}\sigma^2)\Delta t + \sigma \sqrt{\Delta t}\, Z\right)
$$

where μ is the drift, σ is volatility, and Z is a normally distributed random variable.

**Market Impact:**  
The model incorporates supply and demand by adjusting the price based on net order volume:

$$
\text{Impact} = \lambda \cdot \frac{\text{Net Volume}}{\text{Daily Volume}}
$$

where λ represents price sensitivity to trading volume.

#### 2. Investor Intelligence & Strategies

The `Strategy` and `Wallet` components manage how traders interact with the market.

- **Risk Clustering:** Shares are categorized into Low, Medium, and High risk groups based on volatility.
- **Strategy Pattern:** Through the `FundsDistributorManager`, traders can employ different allocation logics:
  - **Equal Distribution:** Capital is spread evenly across selected assets.
  - **Markov Chain Logic:** A predictive strategy using transition matrices (Bear vs. Bull states) to estimate the probability of future price increases.

#### Example of the Simulation for 80 days: 
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
