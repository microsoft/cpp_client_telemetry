#!/bin/sh

############################################################
# Help                                                     #
############################################################
Help()
{
   # Display Help
   echo "Run 1DS Test Telemetry Server with specified runtime versions (.net core 3 or .net6)."
   echo
   echo "Syntax: run.sh [-f,--framework|-h, --help]"
   echo "options:"
   echo "-f | --framework     Run server with specified version of runtime. Supported values: netcoreapp3.1 or net6.0. Default value is netcoreapp3.1, if option is not specified"
   echo "-h | --help    Help."
   echo

   exit 0
}

framework="netcoreapp3.1"
while test $# -gt 0;
do
  case "$1" in
    -h|--help) Help;;
    -f|--framework)       
      shift
      if test $# -gt 0; then
        framework=$1
      else
        echo "--framework option value is not specified. Server will be run with $framework"
      fi
      shift
      ;;
    *)  break;;
  esac
done
echo "1DS test telemetry server will be run with $framework"
dotnet run -f $framework

