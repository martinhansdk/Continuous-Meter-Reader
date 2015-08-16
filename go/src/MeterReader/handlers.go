package MeterReader

import (
	"database/sql"
	"github.com/ant0ine/go-json-rest/rest"
	_ "github.com/lib/pq"
	"log"
	"net/http"
)

/*
CREATE TABLE meters
	("id" integer CONSTRAINT pkey PRIMARY KEY,
	 "name" varchar(40) NOt NULL,
	 "unit" varchar(20) NOT NULL,
	 "current_series" integer NOT NULL,
	 "last_count" integer NOT NULL
	 );

CREATE TABLE measurements
	("measured_at" timestamp with time zone,
	 "meter" integer references meters(id),
	 "value" integer
	 );


*/

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

func (mdb *MeterDB) GetMeterState() map[int32]*Meter {
	meters := make(map[int32]*Meter)

	var (
		id             int
		name           string
		unit           string
		current_series int
		last_count     int
		last_value     int
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
	}
	err = rows.Err()
	if err != nil {
		log.Fatal(err)
	}

	return meters
}

func (mdb *MeterDB) InsertMeasurement() {
	// FIXME
}

func (mdb *MeterDB) GetJSONFromDB(w rest.ResponseWriter, query string, args ...interface{}) {
	var json string
	err := mdb.db.QueryRow("SELECT json_agg(t) FROM ("+query+") ) as t;", args...).Scan(&json)

	if err != nil {
		rest.Error(w, err.Error(), http.StatusInternalServerError)
	}

	w.(http.ResponseWriter).Write([]byte(json))
}

func (mdb *MeterDB) GetCurrentAbsoluteValues(w rest.ResponseWriter, req *rest.Request) {
	mdb.GetJSONFromDB(w, `
		SELECT 
			meter,
			name
			unit,
			LAST_VALUE(value) OVER (PARTITION BY meters.id ORDER BY measured_at DESC) AS value 
		FROM measurements, meters
		WHERE meters.id = measurements.meter`)
}

func (mdb *MeterDB) GetCumulativeValues(w rest.ResponseWriter, req *rest.Request) {
	mdb.GetJSONFromDB(w, `
		SELECT 
			extract(epoch FROM measured_at) AS time, 
			value - FIRST_VALUE(value) OVER (ORDER BY measured_at) AS rel_value 
		FROM measurements
		WHERE meter=?`, req.PathParam("meter"))
}

func (mdb *MeterDB) GetDifferentialValues(w rest.ResponseWriter, req *rest.Request) {
	mdb.GetJSONFromDB(w, `
		SELECT 
			extract(epoch FROM measured_at) AS time, 
			value - LAG(value) OVER (ORDER BY measured_at) as diff_value 
		FROM measurements
		WHERE meter=?`, req.PathParam("meter"))
}

func (mdb *MeterDB) GetDummyValues(w rest.ResponseWriter, req *rest.Request) {
	json := `[{	
		"color": "blue",
		"name": "New York",
		"data": [ { "x": 0, "y": 40 }, { "x": 1, "y": 49 }, { "x": 2, "y": 38 }, { "x": 3, "y": 30 }, { "x": 4, "y": 32 } ]	
		}]`
	w.(http.ResponseWriter).Write([]byte(json))
}
