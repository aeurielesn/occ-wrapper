# wrapper #

It's a utility used at [occ], colombian computing olympiad, for testing problem's solutions.

[occ]: http://olimpia.uan.edu.co/olimpiadas/public/frameset.jsp

## Synopsis ##

    wrapper --execute=filename --problem-id=identifier --os=<xp|vista> [OPTIONS]...

## Description ##

`wrapper` is useful to restrict the execution of programs for a period of time in _milliseconds_ unlike [choice] and [ping] MS-DOS commands of Windows systems which let you pause execution for a fixed time only in [seconds][delays] against test cases.

[delays]: http://www.allenware.com/icsw/icswref.htm#WaitsFixed
[choice]: http://www.allenware.com/icsw/icswref.htm#WaitsFixedChoice
[ping]: http://www.allenware.com/icsw/icswref.htm#WaitsFixedPing

`wrapper` allows you to do custom pre-processing such as copy or delete temporal files and input/output files before the main process (`--execute`) execution. In the same way, you can do post-processing after the finalization of the main process.

As a practical matter, `wrapper` helps you to run a batch testing process for a program against a chosen number of test cases and a specified period of time, the program will be forcefully terminated to free system resources when limit time expires.

`wrapper` relies in [WaitForSingleObject] function to time program execution and in `tskill` or `taskkill` for processes termination.

[WaitForSingleObject]: http://msdn.microsoft.com/en-us/library/bb202783.aspx

## Options ##

    --initial-test-case=value, -i value
> set the number of the first test case. Default `value`  is `1`.

    --test-case-step=value, -s value
> set the increment value. Default `value` is `1`.

    --number-of-test-cases=value, -t value
> set the number of test cases to be executed. Default `value` is `10`.

    --pre-execute=filename, -p filename
> set the file to be executed before running `--execute`. This file will be called with the following arguments: `<program identifier> <current test case string>`. Default `filename` is `pre.bat`.

> A default `pre.bat` could be:
>>     del %1.sal
>>     copy %2.ent %1.ent

    --execute=filename, -x filename
> set the file to be executed after the finalization of `--pre-execute`. Generally, it's the compiled source code to be run against the test cases.

    --post-execute=filename, -P filename
> set the file to be executed after the finalization of `--execute`. This file will be called with the following arguments: `<program identifier> <current test case string>`. Default `filename` is `post.bat`.

> A default `post.bat` could be:
>>     fc %1.sal %2.sal
>>     if not errorlevel 1 echo %2 YES >> outfile
>>     if errorlevel 1 echo %2 NO >> outfile

    --test-cases-syntax=format, -y format
> set the format string to be used for generating the test cases' filenames. The `format` might be specified as described in [printf].

[printf]: http://www.cppreference.com/wiki/c/io/printf

    --problem-id=identifier, -r identifier
> set the problem identifier. If `--test-cases-syntax` is not specified, `<identifier>%d` will be used as `format` string.

    --os=<xp|vista>, -o <xp|vista>
> set the corresponding Windows' operative system in which `wrapper` will be run. `tskill` or `taskkill` will be used depending on the selection.

    --time-limit=milliseconds, -T milliseconds
> set the time limit for execution of `--execute`. When time limit expires and `--execute` has not completed, the process started by `--execute` will be forcefully terminated. Default `milliseconds` is `1000`.

## Author ##

Written by [Alexander Urieles][aeurielesn].

[aeurielesn]: http://github.com/aeurielesn
