#!/bin/sh
git clone https://github.com/josdejong/jsoneditor.git jsoneditor
cd jsoneditor
git pull
npm -g install browserify
browserify ./index.js -o ./jsoneditor.custom.js -s JSONEditor
cd ..
cp jsoneditor/jsoneditor.custom.js wwwroot
