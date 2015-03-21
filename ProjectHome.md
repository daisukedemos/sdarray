sdarray is  C++ library for managing sparse arrays and intergers.

SDarray uses a succinct data structure originally proposed in [1](1.md). SDarray is different from the original work in the following points;
- Uses a select query only on a fixed-width dense array.
- Uses a engineered select operations.

[1](1.md) "Practical Entropy-Compressed Rank/Select Dictionary", D. Okanohara and K. Sadakane., In the Proceedings of ALENEX 2007