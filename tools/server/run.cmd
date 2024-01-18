@ECHO OFF
set framework="netcoreapp3.1"

:loop
IF NOT "%1"=="" (
    IF "%1"=="-f" (
        SET framework=%2
        SHIFT
    )
    IF "%1"=="--framework" (
        SET framework=%2
        SHIFT
    )
    IF "%1"=="-h" GOTO :help
    IF "%1"=="--help" GOTO :help
    SHIFT
    GOTO :loop
)
echo "1DS test telemetry server will be run with %framework%"
dotnet run --framework %framework%
goto:eof

:help
   echo "Run 1DS Test Telemetry Server with specified runtime versions (.net core 3 or .net6)."
   echo "Syntax: run.sh [-f,--framework|-h, --help]"
   echo "options:"
   echo "-f | --framework     Run server with specified version of runtime. Supported values: netcoreapp3.1 or net6.0. Default value is netcoreapp3.1, if option is not specified"
   echo "-h | --help    Help."
   goto:eof