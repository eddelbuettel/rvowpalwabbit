## RVowpalWabbit: R interface to Vowpal Wabbit

[![CI](https://github.com/eddelbuettel/rvowpalwabbit/workflows/ci/badge.svg)](https://github.com/eddelbuettel/rvowpalwabbit/actions?query=workflow%3Aci)
[![License](https://img.shields.io/badge/license-GPL%20%28%3E=%202%29-brightgreen.svg?style=flat)](https://www.gnu.org/licenses/gpl-2.0.html) 
[![Last Commit](https://img.shields.io/github/last-commit/eddelbuettel/rvowpalwabbit)](https://github.com/eddelbuettel/rvowpalwabbit)

### About

A proof-of-concept wrapping of (an earlier version of) Vowpal Wabbit

### Status

It works. It could do with some help. And it should be updated now to use the external VowpalWabbit
library which did not exists when this work was originally done. Collaborators welcome.

It was a part of CRAN from Sep 2011 to Aug 2023, sort-of grandfathered, and
now archived as it also reflects older code and design choices in the
interface (Boost parse_args as sole input is one, the actual core processing
is all fine).

It can still be installed easily provided the system requirements
(i.e. Boost) are met.

A newer attempt is in https://github.com/rvw-org/rvw. And an updated (at
last) upstream web presence is at https://vowpalwabbit.org.

### Author

Dirk Eddelbuettel

### License

GPL (>= 2)
