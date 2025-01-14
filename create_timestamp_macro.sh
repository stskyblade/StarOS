#! /usr/bin/bash

echo "// Created by create_timestamp_macro.sh" > Configs.h
echo "#define Compilation_datetime \"`date +'%Y-%m-%d %H:%M:%S'`\"" >> Configs.h
