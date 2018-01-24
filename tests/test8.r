#!/usr/bin/r

library(RVowpalWabbit)

## Test 8: predicts on test 7 model
## {VW} -t -i models/0002c.model -d train-sets/0002.dat -p 0002c.predict
test8 <- c("-t",
           "-i", system.file("test", "models", "0002c.model", package="RVowpalWabbit"),
           "-d", system.file("test", "train-sets", "0002.dat", package="RVowpalWabbit"),
           "--cache_file", file.path(tempdir(), "0002c.cache"),
           "-p", file.path(tempdir(), "0002c.predict.tmp"))

res <- vw(test8, quiet=FALSE)
print(res)
