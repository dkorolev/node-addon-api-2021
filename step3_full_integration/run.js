#!/usr/bin/env node

const { demoAsyncIntegration } = require('bindings')('native');

console.log('demoAsyncInteration(): calling the C++ function.');
const promise = demoAsyncIntegration(
  3000,
  (data) => { console.log(`Timer callback from C++: '${data}'.`); },
  (body) => { console.log(`Route callback from C++: '${body}'.`); }
);
console.log('demoAsyncInteration(): the promise is returned from C++.');
console.log('demoAsyncInteration(): to terminate, wait for 5 seconds, and call `curl -X DELETE localhost:3000`.');
promise.then((value) => { console.log(`demoAsyncInteration(): the promise resolved, from C++, to ${value}.`); });
