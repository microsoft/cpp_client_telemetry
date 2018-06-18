package EventLogger
// #cgo CFLAGS: -DGENERIC_VALUE=32 -I/usr/local/include/aria
// #cgo CXXFLAGS: -std=c++11 -I/usr/local/include/aria
// #cgo LDFLAGS: -L/usr/local/lib/x86_64-linux-gnu -laria -lcurl -lz -lsqlite3 -ldl
import "C"
