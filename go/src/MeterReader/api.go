package MeterReader

import (
	"github.com/ant0ine/go-json-rest/rest"
	"log"
)

func MakeAPI() *rest.Api {
	mdb := NewMeterDB()

	api := rest.NewApi()
	api.Use(rest.DefaultDevStack...)

	router, err := rest.MakeRouter(
		rest.Get("/message", func(w rest.ResponseWriter, req *rest.Request) { w.WriteJson(map[string]string{"Body": "Hello World!"}) }),
		rest.Get("/cumulativevalues/#meter", mdb.GetCumulativeValues),
		rest.Get("/differentialvalues/#meter", mdb.GetDifferentialValues),
	)
	if err != nil {
		log.Fatal(err)
	}
	api.SetApp(router)

	return api
}
