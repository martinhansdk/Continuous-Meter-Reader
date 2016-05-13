package MeterReader

import (
	"database/sql"
	"github.com/ant0ine/go-json-rest/rest"
	_ "github.com/lib/pq"
	"log"
	"net/http"
)

type MeasurementStorer interface {
	InsertMeasurement(msg *MeterUpdate)
	UpdateMeterState(meter *Meter)
}

type PostgresMeterDB struct {
	db *sql.DB
}

func NewMeterDB() *PostgresMeterDB {
	mdb := new(PostgresMeterDB)

	db, err := sql.Open("postgres", "user=meter dbname=meter password=meter2")
	if err != nil {
		log.Fatal(err)
	}
	mdb.db = db

	return mdb
}

func (mdb *PostgresMeterDB) GetMeterState() map[uint32]*Meter {
	meters := make(map[uint32]*Meter)

	var (
		id             uint32
		name           string
		unit           string
		current_series uint32
		start_count    uint64
		last_value     uint64
	)

	rows, err := mdb.db.Query(`
			SELECT DISTINCT ON(meter)
			    id, 
				name, 
				unit, 
				current_series, 
				last_count AS start_count, 
				value AS last_value 
			FROM meters, measurements 
			WHERE meters.id = measurements.meter
			ORDER BY meter, measured_at DESC;`)
	if err != nil {
		log.Fatal(err)
	}
	defer rows.Close()
	for rows.Next() {
		err := rows.Scan(&id, &name, &unit, &current_series, &start_count, &last_value)
		if err != nil {
			log.Fatal(err)
		}
		log.Printf("meter=%d %s/%s, series=%d, start_count=%d, last_value=%d", id, name, unit, current_series, start_count, last_value)
		meter := &Meter{MeterId: id, Name: name, Unit: unit, CurrentSeries: current_series, StartCount: start_count, LastCount: last_value}
		meters[id] = meter
	}
	err = rows.Err()
	if err != nil {
		log.Fatal(err)
	}

	log.Println(meters)
	return meters
}

func (mdb *PostgresMeterDB) UpdateMeterState(meter *Meter) {
	_, err := mdb.db.Exec(`
			UPDATE meters 
			SET current_series=($1), last_count=($2)
			WHERE id=($3);`, meter.CurrentSeries, meter.StartCount, meter.MeterId)

	if err != nil {
		log.Fatal(err)
	}
}

func (mdb *PostgresMeterDB) InsertMeasurement(msg *MeterUpdate) {
	_, err := mdb.db.Exec(`
			INSERT INTO measurements(meter, measured_at, value)
			VALUES(($1), ($2), ($3));`, msg.MeterId, msg.MeasuredAt, msg.Value)

	if err != nil {
		log.Fatal(err)
	}

}

func (mdb *PostgresMeterDB) GetJSONFromDB(w rest.ResponseWriter, query string, args ...interface{}) {
	var json string
	err := mdb.db.QueryRow("SELECT json_agg(t) FROM ("+query+") as t;", args...).Scan(&json)

	if err != nil {
		rest.Error(w, err.Error(), http.StatusInternalServerError)
	}

	w.(http.ResponseWriter).Write([]byte(json))
}

func (mdb *PostgresMeterDB) GetCurrentAbsoluteValues(w rest.ResponseWriter, req *rest.Request) {
	mdb.GetJSONFromDB(w, `
		SELECT DISTINCT ON(meter)
				meter,
				name, 
				unit, 
				value*scale as value
			FROM meters, measurements 
			WHERE meters.id = measurements.meter
			ORDER BY meter, measured_at DESC
		`)
}

func (mdb *PostgresMeterDB) GetValues(w rest.ResponseWriter, req *rest.Request) {
	mdb.GetJSONFromDB(w, `
		SELECT 
		       json_agg(row)
		FROM (
		       SELECT 
  		             extract('epoch' FROM measured_at)*1000 AS x, 
			     value*scale AS y 
                       FROM measurements, meters
                       WHERE measurements.meter=$1 AND meters.id=measurements.meter 
                ) AS row		
		`, req.PathParam("meter"))
}

func (mdb *PostgresMeterDB) GetDifferentialValues(w rest.ResponseWriter, req *rest.Request) {
	mdb.GetJSONFromDB(w, `
		SELECT 
			extract(epoch FROM measured_at) AS time, 
			value - LAG(value) OVER (ORDER BY measured_at) as diff_value 
		FROM measurements
		WHERE meter=$1`, req.PathParam("meter"))
}
