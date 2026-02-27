cmake_minimum_required(VERSION 3.10)

project(TradingSim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)

# ==========================================
# ### NEW: PROFILING CONFIGURATION ###
# ==========================================
# Define flags for when "Profiling" mode is selected
# -pg: Enables gprof
# -O3: Maximum optimization (REQUIRED to measure real performance, not debug)
# -g:  Debug information (necessary for gprof to see function names)
set(CMAKE_CXX_FLAGS_PROFILING "-pg -O3 -g")
set(CMAKE_EXE_LINKER_FLAGS_PROFILING "-pg")

# If build type is not specified, default to Release (optional)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
# ==========================================

set(SOURCE_FILES 
    src/main.cpp
    src/core/Market.cpp
    src/core/Share.cpp
    src/core/Wallet.cpp         
    src/core/Trader.cpp
    src/core/Strategy.cpp
    src/core/PriceModel.cpp      
    src/core/DataLoader.cpp
    src/core/FundDistributorStrategies.cpp
    src/core/FundsDistributorManager.cpp
)

add_executable(TradingApp ${SOURCE_FILES})

if(CMAKE_BUILD_TYPE STREQUAL "Profiling")
    target_compile_options(TradingApp PUBLIC "-pg")
    target_link_libraries(TradingApp PUBLIC "-pg")
endif()

target_include_directories(TradingApp PRIVATE 
    include                
    include/trading/core  
    external             
)
