#!/usr/bin/r

library(RVowpalWabbit)

# Test 5: add -q .., adaptive, and more (same input, different outputs)
# {VW} --initial_t 1 --power_t 0.5 --adaptive -q Tf -q ff -f models/0002a.model train-sets/0002.dat
test5 <- c("--initial_t", "1",
           "--power_t", "0.5",
           "--adaptive",
           "-q", "Tf",
           "-q", "ff",
           "-f", file.path(tempdir(), "0002a.model"),
           "--cache_file", file.path(tempdir(), "0002a.cache"),
           system.file("test", "train-sets", "0002.dat", package="RVowpalWabbit"))

res <- vw(test5, quiet=FALSE)
print(res)
