#!/usr/bin/r

library(RVowpalWabbit)

# t 2: checking predictions as well
# {VW} -t train-sets/0001.dat -i models/0001.model -p 001.predict.tmp
test2 <- c("-t", system.file("test", "train-sets", "0001.dat", package="RVowpalWabbit"),
           "-i", system.file("test", "models", "0001.model", package="RVowpalWabbit"),
           "--cache_file", file.path(tempdir(), "0001.cache"),
           "-p", file.path(tempdir(), "0001.predict.tmp"))

res <- vw(test2, quiet=FALSE)
print(res)
