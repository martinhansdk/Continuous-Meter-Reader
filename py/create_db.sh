# keep values for every second for a day
# keep values for every minute for a month
# keep values for every day for 10 years
 rrdtool create meter.rrd --step=1 DS:water:COUNTER:120:U:U RRA:AVERAGE:0.5:1:86400 RRA:AVERAGE:0.5:60:43800 RRA:AVERAGE:0.5:86400:3650
