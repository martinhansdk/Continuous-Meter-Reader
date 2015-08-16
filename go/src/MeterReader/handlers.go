package MeterReader

import (
	"database/sql"
	"github.com/ant0ine/go-json-rest/rest"
	_ "github.com/lib/pq"
	"log"
	"net/http"
)

/*
CREATE TABLE measurements
	("measured_at" timestamp with time zone,
	 "meter" int,
	 "value" int)
;
*/

type MeterDB struct {
	db *sql.DB
}

func NewMeterDB() *MeterDB {
	mdb := new(MeterDB)

	db, err := sql.Open("postgres", "user=martin dbname=meters password=meter2")
	if err != nil {
		log.Fatal(err)
	}
	mdb.db = db

	return mdb
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
