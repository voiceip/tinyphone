# Building for macOS

```bash
#install dependencies
brew install autoconf automake libtool opencore-amr cryptopp

#build dependencies

#boost
cd tinyphone-osx/vendor/boost
./boost.sh -macos --boost-version 1.68.0

#statsd
cd tinyphone/lib/statsd-cpp/build-osx
cmake ..
make

#lets install the pods
pod install

#now open the `Tinyphone.xcworkspace` file and compile!
```

