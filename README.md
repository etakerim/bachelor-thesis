# Bachelor's thesis

*Won dean's award for an excellent bachelor's thesis in year 2021/22*

- **Title (SK)**: Spracovanie dát generovaných senzorovou IoT sieťou
- **Title (EN)**: Data processing for sensor IoT network
- **University**: Faculty of Informatics and Information Technologies, Slovak University of Technology in Bratislava
- **Author**: Miroslav Hájek
- **Supervisor**: Ing. Marcel Baláž, PhD.
- **Opponent**: Ing. Vladimír Kunštár
- **Year**: June 2022
- **Language**: Slovak
- **Pages**: 64
- **Thesis registry**: https://opac.crzp.sk/?fn=detailBiblioForm&sid=4F55101800366CEF5FF3C1DB109A
---

## Abstract

An increasing number of connected IoT devices often rely on the aggregation and analytical processing of produced data by more powerful nodes, unnecessarily wasting the goodput of the wireless line. Created pipeline acts directly at the source of the data by adjusting measured levels of vibrational signal obtained during transport from the MEMS three-axis accelerometer. In the time domain, the reduction is achieved by descriptive statistics in the sliding windows. We compare simple peak-finding algorithms that reveal significant harmonic components in the frequency domain. We also come up with a streaming algorithm to detect changes in the frequency component. We test the acquired efficiency by firmware implementation on the ESP32 microcontroller. It enables remote configuration and message sending in the Message Pack format via the MQTT protocol and WiFi. The detector is able to identify important frequencies in custom public transport vehicle datasets with manually tuned parameters according to reference from synthetically generated spectral profiles.

**Keywords**: IoT, vibration monitoring, data processing, edge computing


## Assignment

Sensor IoT networks have become a common part of various industrial processes. Their primary role is collection of various data from the environment, their storage and evaluation in real time. Analyzing and the evaluation of data from only a small number of sensors during continuous monitoring is a big one
challenge. Sensors produce a large amount of data and anomalies may not be detectable at first glance. The goal of the project is to analyze the data captured by the sensor network. Analyze algorithms for their storage and processing. Analyze the individual levels of the sensor network and identify the places where the data could be given partially process. Based on the analysis, propose a method of data storage and processing, or optimization data flow for an existing sensor network. Implement your solution and test its functionality.



## Digital medium

- `firmware` - Source code of the firmware of the sensor unit
	- `esp-dsp` - ESP DSP Library - a library for optimizing the processing of signals in ESP32.
	- `esp-idf` - Espressif IoT Development Framework - SDK for hardware
ESP32.
	- `mpack` - MPack encoder and decoder library MessagePack serializer format.
	- `vibration-analyzer` - The application logic implementation itself.
		- `conf` - Configuration files for OpenLog (config.txt), MQTT broker (mosquitto.conf ) and to compile the documentation via Doxygen (doxygen.conf).
		- `docs` - Doxygen documentation. The home page is index.html.
		- `main`
			- `include` - Application .h header files.
			- `src` - Source files .c application.
		- tests - Unit tests to check the most important functionality and validating the consistency between Python and C implementation of the algorithms: test_events.c (test of the current algorithm for finding the change in frequencies), test_peaks.c (test of peak classifiers), test_config.py (test
remote device configuration). A tool for interactive remote access to IoT unit config_tool.py.
- `measurements` - Analysis of data sets and generation of synthetic signals
in notebooks .ipynb.
	- `datasets` - Vibration records from public transport vehicles.
	- `experiments` - Original monitoring statements from experiments serving as a treasure to verify the solution.
	- `accuracies` - Classification successes on synthetic spectral profile. The results.txt file. based on analysis in VibrationProcessingAlgorithms.ipynb, it got into tabular form in results-table.csv.
	- `execution-algorithms` - Execution times of individual algorithms.
	- `execution-pipeline` - Execution times of data processing changes
	- `memory-usage` - Consumption of flash memory.
	- `network` - Captured network communication from MQTT broker in .pcap files.
	- `signals` - Useful functions for exploratory analysis, visualization and creation models in Jupyter notebooks.

