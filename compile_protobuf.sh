#!/bin/bash

protoc --plugin=protoc-gen-go=go/bin/protoc-gen-go --go_out=go/src/MeterReader/ MeterReader.proto

## generate code for embedded C and fix the names so the Makefile can find them
protoc --plugin=protoc-gen-nanopb=/home/martin/code/nanopb/generator/protoc-gen-nanopb --nanopb_out=lib/MeterReader_pb MeterReader.proto
mv lib/MeterReader_pb/MeterReader.pb.h  lib/MeterReader_pb/MeterReader_pb.h 
mv lib/MeterReader_pb/MeterReader.pb.c  lib/MeterReader_pb/MeterReader_pb.c
perl -pni -e 's/"MeterReader.pb.h"/"MeterReader_pb.h"/' lib/MeterReader_pb/MeterReader_pb.c
