#!/bin/bash
## 
## Check for writes back into package directory
## Thanks to Kurt Hornik for the bindfs/fusermount suggestion
##
## customized for my directory layout, and current test version
## adjust as needed
lib=/usr/local/lib/R-devel/lib/R/library/
pkg=RVowpalWabbit_0.0.11.1.tar.gz
cd ~/git/rvowpalwabbit

## RD (aka R-devel) only knows one installation directory: ${lib}
RD CMD INSTALL ${pkg}

## Now mount ${lib} read-only to catch writes there
sudo bindfs -r ${lib} ${lib}

## Check (twice even)
RD CMD check --install=skip ${pkg}
RD -e 'tools::testInstalledPackage("RVowpalWabbit")'

## Unmount
sudo fusermount -u ${lib}
