#!/usr/bin/r

library(RVowpalWabbit)

# Test 6: run predictions on Test 4 model
# Pretending the labels aren't there
# {VW} -t -i models/0002.model -d train-sets/0002.dat -p 0002b.predict
test6 <- c("-t", "-i", system.file("test", "models", "0002.model", package="RVowpalWabbit"),
           "-d", system.file("test", "train-sets", "0002.dat", package="RVowpalWabbit"),
           "--cache_file", file.path(tempdir(), "0002b.cache"),
           "-p", file.path(tempdir(), "0002b.predict.tmp"))

res <- vw(test6, quiet=FALSE)
print(res)
