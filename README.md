# naive-garble

[Garbled circuit](https://en.wikipedia.org/wiki/Garbled_circuit) is a cryptography protocol that facilitates secured function evalutation (SFE).
This site demonstrates the basics of it and can be used for SFE in limited scenarios (semi-trusted).

## Online

See [Demo](https://garbled.netlify.com/) in WebAssembly.

## Run Locallly

```sh
make -j4
./bin/garble --alice 1  # 0~3
./bin/garble --bob 2    # 0~3
# Should evaluate to min(1, 2) = 1
```
