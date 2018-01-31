#!/usr/bin/r

library(RVowpalWabbit)

# Test 7: using -q and multiple threads
# {VW} --adaptive -q ff -f models/0002c.model train-sets/0002.dat
test7 <- c("--adaptive", "-q", "ff",
           "-f", file.path(tempdir(), "0002c.model"),
           "--cache_file", file.path(tempdir(), "0002c.cache"),
           system.file("test", "train-sets", "0002.dat", package="RVowpalWabbit"))

res <- vw(test7, quiet=FALSE)
print(res)
