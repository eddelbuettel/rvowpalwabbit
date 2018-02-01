\name{vw}
\alias{vw}
\title{Run the Vowpal Wabbit fast out-of-core learner}
\description{
  The \code{vw} function applies the Vowpal Wabbit on-line learner
  to a given data set and model.

  Vowpal Wabbit is a project sponsored by Yahoo! Research and led by
  John Langford.

  At present, this package provides a simple yet crude interface.
}
\usage{
vw(args, quiet=TRUE)
}
\arguments{
  \item{args}{A character vector containing the same arguments one would
    use on the command-line with the standalone \code{vw} binary.}
  \item{quiet}{A boolean switch which, if set, suppresses most output to
    stdout.}
}
\value{
  The \code{vw} returns a small data.frame with a number of summary
  statistics function returns a character string of a fixed
  length containing the requested digest of the supplied R object. For
  MD5, a string of length 32 is returned; for SHA-1, a string of length
  40 is returned; for CRC32 a string of length 8.
}
\details{
  Vowpal Wabbit is a very fast on-line machine learning
  application. Some documentation for it is provided via the upstream
  wiki referenced below.
}
\note{The \pkg{RVowpalWabbit} package contains the original test and
  validattion data, models, and output. In order to call the \code{vw}
  function with relative path names (as in the Vowpal Wabbit
  documentation), it is easiest to first change to a directory above all
  these files as for example via

  \preformatted{
    ## change to 'test' directory of package
    setwd( system.file("test", package="RVowpalWabbit") )
  }

  which computes where the package is installed, and then adds the
  \code{test} directory to that path before changing to working
  directory to the resulting path.
}
\references{
  \url{https://github.com/JohnLangford/vowpal_wabbit/wiki}
}
\author{Dirk Eddelbuettel \email{edd@debian.org} for the \R interface;
  John Langford along with Daniel Hsu, Nikos Karampatziakis, Olivier
  Chapelle, Paul Mineiro, Matt Hoffman, Jake Hofman, Sudarshan Lamkhede,
  Shubham Chopra, Ariel Faigon, Lihong Li, Gordon Rios, and Alex Strehl
  for Vowpal Wabbit.
} 
\examples{
  ## also see demo(vw) from which this is a subset

  library(RVowpalWabbit)

  # Test 3: without -d, training only
  # {VW} train-sets/0002.dat    -f models/0002.model
  test3 <- c("-t", system.file("test", "train-sets", "0002.dat", package="RVowpalWabbit"),
             "-f", file.path(tempdir(), "0002.model"),
             "--cache_file", file.path(tempdir(), "0002.cache"))

  res <- vw(test3)
  res
}
\keyword{misc}

