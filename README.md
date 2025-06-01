# Overview
This project transforms the M5Paper into a display for real-time SBB train departure schedules. 
<div>
  <img src="https://github.com/user-attachments/assets/38729050-f935-42e1-8e71-636a6beda1f9" width=30%>
</div>

# Getting Started
Make sure you don't spam the API!!! Your IP will be blocked.   
https://transport.opendata.ch/docs.html#rate-limiting  
Keep in mind this code is quite ugly and potentially full of bugs! It is a PoC... (: 

## Prerequisite
- M5Paper device
- PlatformIO (+ add board etc..)
- M5EPD Library

## Setup
Note the following lines in `crawler.cpp`:
```C++
#include "credentials.h"
...
WiFi.begin(network::WIFI_SSID, network::WIFI_PW);
...
```
First you have to manually create a file `credentials.h`, where you define your WIFI credentials:
```C++
#pragma once
namespace network {
    const char * WIFI_SSID = "YOUR SSID";
    const char * WIFI_PW = "YOUR PASSWORD";
};
```

Currently, you have to manually edit the code to change/add/remove connections.

## Code
- The `Crawler` class is responsible to handle HTTP requests for us.
- `API` namespace provides functions to create, parse and print responses from the API
  currently it also creates the sub-canvas to display connection information
- All the other functionality is defined in the main file.
  
# Todo
- create new Class for handling displaying the information 
  - move canvas creation out of the `API` namespace, as it has nothing to do with the API
  - move display updates out of the `main.cpp` file
  - let it handle generic connections
  - move sensor information out of the `main.cpp` file
- improve handling of timers
- optimize power consumption
- add functionality
