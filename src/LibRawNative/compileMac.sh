g++ -shared LibRaw.cpp -I /Users/hs/LibRaw-0.20.2/ /Users/hs/LibRaw-0.20.2/lib/libraw.a -o libLibRawNative.dylib
cp libLibRawNative.dylib ../../bin/Debug/netcoreapp3.1 
cp libLibRawNative.dylib ../../bin/Debug/netstandard2.0
cp libLibRawNative.dylib ../../bin/Release/netcoreapp3.1 
cp libLibRawNative.dylib ../../bin/Release/netstandard2.0
cp libLibRawNative.dylib ../../lib/Native/LibRawSharp/mac/AMD64
