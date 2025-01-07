sqlite3 ~/.avidemux6/jobs.sql .d > dump
#sql2class -sqlite -global -license -wrapped  -lib $PWD dump
sql2class -sqlite -build -global -prefix $PWD -lib sqlJobs -namespace db -overwrite dump
