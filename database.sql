CREATE TABLE meters
	("id" integer CONSTRAINT pkey PRIMARY KEY,
	 "name" varchar(40) NOT NULL,
	 "unit" varchar(20) NOT NULL,
	 "current_series" integer NOT NULL,
	 "last_count" integer NOT NULL,
	 "scale" real NOT NULL
	 );

CREATE TABLE measurements
	("measured_at" timestamp with time zone,
	 "meter" integer references meters(id),
	 "value" integer
	 );
