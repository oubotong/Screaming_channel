.. _nrf5340_app_core:

nRF5340: Bitcount firmware for application core inside TEE
############################################

.. contents::
   :local:
   :depth: 2

This sample can be used to run an application on the network core of the nRF5340 when there is no need for the working application core.

Overview
********

The sample has minimal Zephyr configuration: no multithreading, no clock, no MPU, no device drivers.
It does the following things:

* In the `main` function of the sample:

  * Wait 10s to ensure network core has already started
  * Run simple bitcount example as the benchmark and loop it for 20 times.

Requirements
************

The sample supports the following development kit:

.. table-from-rows:: /includes/sample_board_rows.txt
   :header: heading
   :rows: nrf5340dk_nrf5340_cpuapp

Building and running
********************
Before building this example, you need to install `nrf-connect-sdk(https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html)` first

Put this folder under the path ``nrf/samples/nrf5340` in the nrf-connect-sdk``

Then use command to build it: ``west build -b nrf5340dk_nrf5340cpuapp``

Testing
=======
1. Program this sample to the application core by ``west flash``
2. Program cpunet firmware on the network core
3. Turn on SDR and tune to 2.336Ghz to see the changes


