"""
.. module:: dlh

**********
DLH Module
**********

.. _datasheet: http://www.allsensors.com/cad/DS-0355_Rev_B.PDF

This module contains the Zerynth driver for Amphenol DLH pressure sensors series. These calibrated and compensated sensors provide accurate, stable output over a wide temperature range. This series is intended for use with non-corrosive, non-ionic working fluids such as air, dry gases and the like.

The driver implements SPI communication.

"""

import spi


MODE_SINGLE = 0
MODE_AVG2  = 1
MODE_AVG4  = 2
MODE_AVG8  = 3
MODE_AVG16 = 4

TYPE_D = 0
TYPE_G = 1

UNIT_INH20  = 0
UNIT_CMH20  = 1
UNIT_PASCAL = 2


@native_c("_dlh_getfast",["csrc/dlh.c"],["VHAL_SPI"],[])
def dlh_getfast(spidrv,mode,unit,d_or_g):
    pass



class DLH(spi.Spi):
    """
===============
 DLH Class
===============

.. class:: DLH(spidrv, cs, d_or_g, clock=2000000)

    Creates an intance of the DLH class.

    :param spidrv: SPI Bus used '( SPI0, ... )'
    :param cs: GPIO to be used as Carrier Select
    :param d_or_g: specifies DLH type D (differential) or G (absolute) sensor. can be one of :samp:`TYPE_D` or :samp:`TYPE_G` constants
    :param clock: spi clock to be used

    Temperature and pressure values can be easily obtained from the sensor: ::

        from amphenol.dlh import dlh

        ...

        d = dlh.DLH(SPI0, D10, dlh.TYPE_D)

        press, temp = d.get_values(unit=dlh.UNIT_PASCAL)

    """
    def __init__(self, spidrv, cs, d_or_g, clock=2000000):
        self.d_or_g = d_or_g
        spi.Spi.__init__(self,cs, spidrv, clock)
        self._start()

    def get_values(self,mode=MODE_SINGLE,unit=UNIT_INH20):
        """
    .. method:: get_values(mode=MODE_SINGLE, unit=UNIT_INH20)

        Return a 2-element tuple containing current pressure and temperature values.
        The acquisition mode can be specified with one of:

        * :samp:`MODE_SINGLE`
        * :samp:`MODE_AVG2`
        * :samp:`MODE_AVG4`
        * :samp:`MODE_AVG8`
        * :samp:`MODE_AVG16`

        The unit of measure of pressure can be specified in:

        * :samp:`UNIT_INH20`
        * :samp:`UNIT_CMH20`
        * :samp:`UNIT_PASCAL`

        """
        res = dlh_getfast(self.drvid,mode,unit,self.d_or_g)
        return (res[0]/1000000,res[1]/1000)






