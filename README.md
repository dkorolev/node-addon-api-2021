## Experimenting with `node-addon-api` in 2021

### Step 1: No External Dependency

This is a trivial example that actually checks quite a lot, most importantly, whether you have the right node/npm/C++ compiler, and whether they play well together.

```
(cd step1_no_external_dependency && npm i && ./run.js)
```

Should print, after all the `npm i` output:
```
The Magic happened.
```

#### Details

The string that is output is coming from a C++ source file. This source is rebuilt by running `npm i`. To verify this, from the `step1_no_external_dependency` directory:

1. Edit the C++ source file, `sed -i 's/happened/happened again/' native.cc`.
2. Observe that just `./run.js` prints the old message.
3. Confirm that, after `npm i`, `./run.js` prints the updated one, QED.

### Step 2: Using an External Dependency

This step confirms that it is easy to link an external `.so` to the above process. TL;DR:

```
(cd step2_external_dependency && (cd extension && make) && npm i && ./run.js)
```

Should print, as the last line:
```
The Magic from an external `.so` library just happened.
```

The importance of this step is that it can link an `.so` that was built by means of a very different build process, which might be tricky to befriend with the `node-gyp`'s "native" build process.

#### Details

The string that is output is coming from a C++ code, but its very contents is populated by another piece of C++ code. The former C++ code is the `native.cc` source file, and the latter one is the `extension/extension.cc` file; the former is built via `node-gyp` with `npm i`, and the latter it built by `make` + `g++` via `(cd extension && make)`. To get a feel, from the `step2_external_dependency` directory:

1. Edit the inner source file, `sed -i 's/The Magic/Even More Magic/' extension/extension.cc`.
2. Observe that just `./run.js` prints the old message.
3. Observe that `npm i && ./run.js` prints the old message.
4. Run `(cd extension && make)`, which would rebuild the `.so` that is picked up by `npm i`.
5. At this point, before `npm i`, `./run.js` would still print the old message.
6. Confirm that now, after `npm i`, `./run.js` prints the updated message, QED.

### Step 3: Asynchronously Calling JavaScript Functions from C++

This step confirms that an external `.a` library can invoke JavaScript callbacks. TL;DR:

```
([ -d ../current ] || (cd ..; git clone https://github.com/c5t/current)) && (cd step3_full_integration && (cd extension && make) && make)
```

The above command, after the build, would start two "workers", both in C++.

One is the timer thread, and it would run for five seconds, printing integers to the console via a JavaScript callback function every 500ms.

The other one is an HTTP server, on `localhost:3000` by default, and it needs a `DELETE` request to shut itself down. All HTTP requests, including the very DELETE one(s), are logged to the console, via another JavaScript callback.

To stop the second "worker" then, from a separate terminal, issue `curl -X DELETE localhost:3000`. The very test script terminates once both threads are done; in other words, this example script runs until the HTTP server is DELETE-d, but for no less than five seconds in total.
