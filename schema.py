import sqlite3, os, sys
p=sys.argv[1]
c=sqlite3.connect(p)
for row in c.execute("select name, type from sqlite_master where type in ('table','view') order by name"):
    print(row)
    if row[1]=='table':
        print('  ', [r[1] for r in c.execute(f"pragma table_info({row[0]})")])
