# @echo off

# set VSTOOLS_VERSION=vs2019
# cd %~dp0

# call ..\..\tools\vcvars.cmd
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#deploy-dll.cmd
#generate-bindings.cmd

cargo run