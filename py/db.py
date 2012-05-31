from pyrrd.rrd import DataSource, RRA, RRD

class DB(object):
    
    def __init__(self):
        self.filename = 'meter.rrd'
        self.rrd = RRD(self.filename,
                       )
        self.rrd.load()

    def add(timestamp, value):
        self.rrd.bufferValue(str(timestamp), str(value))
        self.rrd.update()
