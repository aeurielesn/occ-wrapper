fc %1.sal %2.sal
if not errorlevel 1 echo %2 YES >> outfile
if errorlevel 1 echo %2 NO >> outfile
