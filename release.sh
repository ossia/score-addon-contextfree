#!/bin/bash
rm -rf release
mkdir -p release

cp -rf ContextFree *.{hpp,cpp,txt,json} LICENSE release/

mv release score-addon-contextfree
7z a score-addon-contextfree.zip score-addon-contextfree
