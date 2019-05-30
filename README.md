# naive-garble

## Online

See [Demo](https://garbled.netlify.com/) in WebAssembly.

## Run Locallly

```sh
make -j4
./bin/garble --alice 1  # 0~3
./bin/garble --bob 2    # 0~3
# Should evaluate to min(1, 2) = 1
```

## What & Why

[Wikipedia](https://en.wikipedia.org/wiki/Garbled_circuit).
