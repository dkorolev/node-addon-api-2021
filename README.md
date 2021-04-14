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
