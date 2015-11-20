package MeterReader

import (
	"database/sql"
	"github.com/ant0ine/go-json-rest/rest"
	_ "github.com/lib/pq"
	"log"
	"net/http"
)

type MeterDB struct {
	db *sql.DB
}

func NewMeterDB() *MeterDB {
	mdb := new(MeterDB)

	db, err := sql.Open("postgres", "user=meter dbname=meter password=meter2")
	if err != nil {
		log.Fatal(err)
	}
	mdb.db = db

	return mdb
}

func (mdb *MeterDB) GetMeterState() map[uint32]*Meter {
	meters := make(map[uint32]*Meter)

	var (
		id             uint32
		name           string
		unit           string
		current_series uint32
		last_count     uint64
		last_value     uint64
	)

	rows, err := mdb.db.Query(`
			SELECT DISTINCT ON(meter)
			    id, 
				name, 
				unit, 
				current_series, 
				last_count, 
				value AS last_value 
			FROM meters, measurements 
			WHERE meters.id = measurements.meter
			ORDER BY meter, measured_at DESC;`)
	if err != nil {
		log.Fatal(err)
	}
	defer rows.Close()
	for rows.Next() {
		err := rows.Scan(&id, &name, &unit, &current_series, &last_count, &last_value)
		if err != nil {
			log.Fatal(err)
		}
		log.Println(id, name, unit, current_series, last_count, last_value)
		meter := &Meter{MeterId: id, Name: name, Unit: unit, CurrentSeries: current_series, StartCount: last_count, LastCount: last_value}
		meters[id] = meter
	}
	err = rows.Err()
	if err != nil {
		log.Fatal(err)
	}

	log.Println(meters)
	return meters
}

func (mdb *MeterDB) InsertMeasurement(msg *MeterUpdate) {
	_, err := mdb.db.Exec(`
			INSERT INTO measurements(meter, measured_at, value)
			VALUES(($1), ($2), ($3));`, msg.MeterId, msg.MeasuredAt, msg.Value)

	if err != nil {
		log.Fatal(err)
	}

}

func (mdb *MeterDB) GetJSONFromDB(w rest.ResponseWriter, query string, args ...interface{}) {
	var json string
	err := mdb.db.QueryRow("SELECT json_agg(t) FROM ("+query+") as t;", args...).Scan(&json)

	if err != nil {
		rest.Error(w, err.Error(), http.StatusInternalServerError)
	}

	w.(http.ResponseWriter).Write([]byte(json))
}

func (mdb *MeterDB) GetCurrentAbsoluteValues(w rest.ResponseWriter, req *rest.Request) {
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

func (mdb *MeterDB) GetValues(w rest.ResponseWriter, req *rest.Request) {
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

func (mdb *MeterDB) GetDifferentialValues(w rest.ResponseWriter, req *rest.Request) {
	mdb.GetJSONFromDB(w, `
		SELECT 
			extract(epoch FROM measured_at) AS time, 
			value - LAG(value) OVER (ORDER BY measured_at) as diff_value 
		FROM measurements
		WHERE meter=$1`, req.PathParam("meter"))
}

func (mdb *MeterDB) GetDummyValues(w rest.ResponseWriter, req *rest.Request) {
	json := `[{	
		"color": "blue",
		"name": "New York",
		"data": [ { "x": 0, "y": 40 }, { "x": 1, "y": 49 }, { "x": 2, "y": 38 }, { "x": 3, "y": 30 }, { "x": 4, "y": 32 } ]	
		}]`
	w.(http.ResponseWriter).Write([]byte(json))
}
