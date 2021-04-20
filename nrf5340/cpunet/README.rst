.. _radio_test:

Radio test
##########

.. contents::
   :local:
   :depth: 2

The Radio test sample demonstrates how to configure the radio in a specific mode and then test its performance.
The sample provides a predefined commands that allow you to configure the radio:

* Constant TX carrier

Overview
********

This applcation runs on the network core of NRF5340DK. It is used to carry constant bluetooth signal at channel 0(2.4Ghz)

Requirements
************

The sample supports the following development kits:

.. table-from-rows:: /includes/sample_board_rows.txt
   :header: heading
   :rows: nrf5340dk_nrf5340_cpunet

   On nRF5340 DK, the sample is designed to run on the network core.


Building and running
********************
Place the whole folder under ``samples/nrf5340``

Use command to build it: ``west build -b nrf5340dk_nrf5340cpunet``

Then flash it: ``west flash`` This will automatically flash it to the network core.
