#!/bin/sh
export TOOLS=`pwd`
export PATH=$PATH:$TOOLS/
echo building JSON schema from CsProtocol.bond ...
cd ../lib/bond/generated
gbc schema ../CsProtocol.bond
echo generating readers/writers...
python $TOOLS/bondjson2cpp.py CsProtocol.json
echo [ DONE ]
