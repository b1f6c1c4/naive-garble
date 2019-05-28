# Naive-garble

## TL;DR

```sh
make -j4
./bin/garble --alice 1
./bin/garble --bob 2
# Should evaluate to min(1, 2) = 1
```

## What

So two semi-trusted parties can collaborately compute a function (here we implemented as table-lookup only) but nobody is going to know what they're supposed to know.
More info please refer to [Wikipedia](https://en.wikipedia.org/wiki/Garbled_circuit).
