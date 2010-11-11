sqlite3 ~/.avidemux6/jobs.sql .d > dump
sql2class -sqlite -prefix -lib /usr dump
