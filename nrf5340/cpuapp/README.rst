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

* In the :c:func:`main` function of the sample:

  * Wait 10s to ensure network core has already started
  * Run simple bitcount example as the benchmark

Requirements
************

The sample supports the following development kit:

.. table-from-rows:: /includes/sample_board_rows.txt
   :header: heading
   :rows: nrf5340dk_nrf5340_cpuapp

Building and running
********************

.. |sample path| replace:: :file:`samples/nrf5340/

Testing
=======
1. Program this sample to the application core.
#. Program Zephyr's :ref:`zephyr:blinky-sample` sample to the network core.
#. Observe the LEDs on the kit.

Dependencies
************

This sample uses the following `nrfx`_ dependencies:

* ``nrfx/nrf.h``
* ``nrfx/nrfx.h``

In addition, it uses the following Zephyr libraries:

* :ref:`zephyr:kernel_api`:

  * ``include/init.h``
