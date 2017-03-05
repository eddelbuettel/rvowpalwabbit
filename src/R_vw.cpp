// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

#include <Rcpp.h>

#include "vw.h"
//#include "gd.h"

// std::string to char* pointer conversion
inline char *string2charptr(const std::string & s) { return const_cast<char*>(s.c_str()); }

// [[Rcpp::export]]
Rcpp::DataFrame Rvw(SEXP args) {

    // use template conversion to transfer the R argument vector in a vector of strings
    std::vector<std::string> vs = Rcpp::as< std::vector<std::string> >(args);
    std::vector<char*> vc;
    std::transform(vs.begin(), vs.end(), std::back_inserter(vc), string2charptr);
    //Rprintf("Fear the Vowpal Wabbit\n");

    gd_vars *vars = vw(vc.size(), &vc[0]);
    
    float weighted_labeled_examples = global.weighted_examples - global.weighted_unlabeled_examples;
    float best_constant = (global.weighted_labels - global.initial_t) / weighted_labeled_examples;
    float constant_loss = (best_constant*(1.0 - best_constant)*(1.0 - best_constant)
                           + (1.0 - best_constant)*best_constant*best_constant);
  
    if (!global.quiet) {
        //std::cerr.precision(4);
        VWCOUT << std::endl << "finished run";
        VWCOUT << std::endl << "number of examples = " << global.example_number;
        VWCOUT << std::endl << "weighted example sum = " << global.weighted_examples;
        VWCOUT << std::endl << "weighted label sum = " << global.weighted_labels;
        VWCOUT << std::endl << "average loss = " << global.sum_loss / global.weighted_examples;
        VWCOUT << std::endl << "best constant = " << best_constant;
        if (global.min_label == 0. && global.max_label == 1. && best_constant < 1. && best_constant > 0.)
            VWCOUT << std::endl << "best constant's loss = " << constant_loss;
        VWCOUT << std::endl << "total feature number = " << global.total_features;
        if (global.active_simulation)
            VWCOUT << std::endl << "total queries = " << global.queries << std::endl;
        VWCOUT << std::endl;
    }
  
    Rcpp::DataFrame df =
        Rcpp::DataFrame::create(Rcpp::Named("numberExamples")     = static_cast<double>(global.example_number),
                                Rcpp::Named("weightedExampleSum") = global.weighted_examples,
                                Rcpp::Named("weightedLabelSum")   = global.weighted_labels,
                                Rcpp::Named("averageLoss") = global.sum_loss / global.weighted_examples,
                                Rcpp::Named("bestConstant") = best_constant,
                                Rcpp::Named("bestConstantsLoss") = (global.min_label == 0. && 
                                                                    global.max_label == 1. && 
                                                                    best_constant < 1. && 
                                                                    best_constant > 0.) ? constant_loss : R_NaReal,
                                Rcpp::Named("totalFeatureNumber") = static_cast<double>(global.total_features),
                                Rcpp::Named("totalQueries") = global.active_simulation ? global.queries : R_NaReal
                                );

    
    free(vars);
    return df;
}
