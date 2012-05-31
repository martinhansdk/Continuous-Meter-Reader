from meter import Meter
from nose.tools import eq_ as eq


def testMeterStartsFromZeroInitially():
    meter = Meter()
    eq(meter.count(),0)

def testMeterCountsFromWhereItLeftOff():
    meter = Meter(initial = 100)
    meter.sample(0)

    eq(meter.count(), 100)

def testMeterCountsFromWhereItLeftOffWhenFirstSampleIsNonZero():
    meter = Meter(initial = 100)
    meter.sample(5)

    eq(meter.count(), 105)

def testMeterCounts():
    meter = Meter(initial = 100)
    meter.sample(0)
    eq(meter.count(), 100)

    meter.sample(1)
    eq(meter.count(), 101)

    meter.sample(5)
    eq(meter.count(), 106)

    meter.sample(1)
    eq(meter.count(), 107)

