package MeterReader

import "testing"

type fakeMeasuementStorer struct {
	stored []MeterUpdate
	meters map[uint32]*Meter
}

func (db *fakeMeasuementStorer) InsertMeasurement(msg *MeterUpdate) {
	db.stored = append(db.stored, *msg)
}

func (db *fakeMeasuementStorer) UpdateMeterState(meter *Meter) {
	m := *meter // shallow copy
	db.meters[meter.MeterId] = &m
}

type fixture struct {
	t      *testing.T
	msh    *MeterStateHandler
	mdb    *fakeMeasuementStorer
	meters map[uint32]*Meter
	sent   []*MeterUpdate
}

func (f *fixture) send(m *MeterUpdate) {
	if m != nil {
		f.sent = append(f.sent, m)
	}
}

func (f *fixture) checkNoStoredMeterState(id uint32) {
	_, ok := f.mdb.meters[id]
	if ok {
		f.t.Error("Stored meter state, but should not have")
	}
}

func (f *fixture) checkStoredMeterState(id, series uint32) {
	meter, ok := f.mdb.meters[id]
	if !ok {
		f.t.Error("Did not store meter state for id ", id)
	} else if meter.CurrentSeries != series {
		f.t.Errorf("Expected meter %d to be stored with series %d, but it is stored with series %d", id, series, meter.CurrentSeries)
	}
}

func (f *fixture) checkNumberOfValuesSent(number int) {

	if len(f.sent) != number {
		f.t.Errorf("Should have sent %d items, but sent:", number, f.sent)
	}

	if len(f.mdb.stored) != number {
		f.t.Fatal("Stored should have stored %d items but stored:", number, f.mdb.stored)
	}
}

func (f *fixture) checkLastValue(val uint64) {
	if len(f.sent) == 0 {
		f.t.Errorf("Should have sent at least 1 item, but none sent")
	}

	if len(f.mdb.stored) == 0 {
		f.t.Errorf("Should have stored at least 1 item, but none stored")
	}

	last_sent := f.sent[len(f.sent)-1].Value
	if last_sent != val {
		f.t.Errorf("Expected last sent value to be %d, got %d", val, last_sent)
	}

	last_stored := f.mdb.stored[len(f.mdb.stored)-1].Value
	if last_stored != val {
		f.t.Errorf("Expected last stored value to be %d, got %d", val, last_stored)
	}
}

func newFixture(t *testing.T) *fixture {
	f := new(fixture)

	f.t = t
	f.msh = new(MeterStateHandler)
	f.meters = make(map[uint32]*Meter) // separate from the mdb.Meters
	f.sent = make([]*MeterUpdate, 0, 10)
	f.mdb = new(fakeMeasuementStorer)

	f.mdb.meters = make(map[uint32]*Meter)
	f.msh.mdb = f.mdb
	f.msh.meters = f.meters

	return f
}

func makeUpdate(MeterId, SeriesId uint32, CurrentCounterValue uint64) *CounterUpdate {
	update := CounterUpdate{MeterId: &MeterId, SeriesId: &SeriesId, CurrentCounterValue: &CurrentCounterValue}
	return &update
}

func TestTranslateIgnoresUnknownMeters(t *testing.T) {
	f := newFixture(t)

	f.send(f.msh.Translate(makeUpdate(1, 5, 100)))
	f.send(f.msh.Translate(makeUpdate(2, 5, 100)))
	f.send(f.msh.Translate(makeUpdate(3, 5, 100)))

	f.checkNumberOfValuesSent(0)

	if len(f.meters) != 0 {
		t.Error("Unknown meters should have been ignored, but in the end the array of meters was:", f.meters)
	}
}

func TestTranslateDoesNotCountUpIfThereIsNoChange(t *testing.T) {
	f := newFixture(t)

	f.meters[1] = &Meter{MeterId: 1, CurrentSeries: 5, LastCount: 100}

	f.send(f.msh.Translate(makeUpdate(1, 5, 100)))
	f.send(f.msh.Translate(makeUpdate(1, 5, 100)))
	f.send(f.msh.Translate(makeUpdate(1, 5, 100)))

	f.checkNumberOfValuesSent(0)
}

func TestTranslateStoresNewSeriesId(t *testing.T) {
	f := newFixture(t)

	f.meters[1] = &Meter{MeterId: 1, CurrentSeries: 5, LastCount: 100}

	f.send(f.msh.Translate(makeUpdate(1, 5, 100)))
	f.send(f.msh.Translate(makeUpdate(1, 6, 0)))
	f.send(f.msh.Translate(makeUpdate(1, 6, 0)))

	f.checkStoredMeterState(1, 6)

	f.checkNumberOfValuesSent(0)
}

func TestTranslateCountsCorrectlyAcrossReboots(t *testing.T) {
	f := newFixture(t)

	f.meters[1] = &Meter{MeterId: 1, CurrentSeries: 5, LastCount: 100}

	f.send(f.msh.Translate(makeUpdate(1, 5, 100)))
	f.send(f.msh.Translate(makeUpdate(1, 6, 0)))
	f.send(f.msh.Translate(makeUpdate(1, 6, 1)))

	f.checkStoredMeterState(1, 6)
	f.checkNumberOfValuesSent(1)
	f.checkLastValue(101)
}

/*
	Scenario leading up the the next test:
	  series=5, count=0, absolute=50
	  MeterServer restarts
	  series=5, count=100, absolute=150
*/
func TestTranslateCountsCorrectlyAcrossWhenServerRebootsButMeterDoesNot(t *testing.T) {
	f := newFixture(t)

	f.meters[1] = &Meter{MeterId: 1, CurrentSeries: 5, LastCount: 50, StartCount: 50}

	f.send(f.msh.Translate(makeUpdate(1, 5, 100)))

	f.checkNoStoredMeterState(1)
	f.checkNumberOfValuesSent(1)
	f.checkLastValue(150)
}

func TestTranslateCountsCorrectlyWhenMeterAlreadyRunningWhenServerStarts(t *testing.T) {
	f := newFixture(t)

	f.meters[1] = &Meter{MeterId: 1, CurrentSeries: 5, LastCount: 100}

	f.send(f.msh.Translate(makeUpdate(1, 6, 10)))

	f.checkStoredMeterState(1, 6)
	f.checkNumberOfValuesSent(1)
	f.checkLastValue(110)
}
