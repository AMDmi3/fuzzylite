/*
 Copyright © 2010-2016 by FuzzyLite Limited.
 All rights reserved.

 This file is part of fuzzylite®.

 fuzzylite® is free software: you can redistribute it and/or modify it under
 the terms of the FuzzyLite License included with the software.

 You should have received a copy of the FuzzyLite License along with 
 fuzzylite®. If not, see <http://www.fuzzylite.com/license/>.

 fuzzylite® is a registered trademark of FuzzyLite Limited.
 */

#ifndef FL_WEIGHTEDSUMCUSTOM_H
#define FL_WEIGHTEDSUMCUSTOM_H


#include "fl/defuzzifier/WeightedDefuzzifier.h"

namespace fl {

    /**
    
      The WeightedSumCustom class is a WeightedDefuzzifier that computes the
      weighted sum of a fuzzy set represented in an Aggregated Term utilizing
      the fuzzy operators for implication and aggregation to perform the weighted
      sum. 

      @author Juan Rada-Vilela, Ph.D.
      @see WeightedSum
      @see WeightedAverage
      @see WeightedAverageCustom
      @see WeightedDefuzzifier
      @see Defuzzifier
      @since 4.0
    
     */
    class FL_API WeightedSumCustom : public WeightedDefuzzifier {
    public:
        explicit WeightedSumCustom(Type type = Automatic);
        explicit WeightedSumCustom(const std::string& type);
        virtual ~WeightedSumCustom() FL_IOVERRIDE;
        FL_DEFAULT_COPY_AND_MOVE(WeightedSumCustom)

        virtual std::string className() const FL_IOVERRIDE;
        /**
          Computes the weighted sum of a fuzzy set represented in an
          Aggregated Term as @f$y = \sum_i{w_iz_i} @f$,
          where @f$w_i@f$ is the activation degree of term @f$i@f$, and @f$z_i
          = \mu_i(w_i) @f$.

          If the implication and aggregation operators are set to fl::null (or 
          set to AlgebraicProduct and UnboundedSum, respectively), then the 
          operation of WeightedAverageCustom is the same as the WeightedAverage.
          Otherwise, the implication and aggregation operators are utilized to 
          compute the multiplications and sums in @f$y$f, respectively.
         
          @param term is the fuzzy set represented as an AggregatedTerm
          @param minimum is the minimum value of the range (only used for Tsukamoto)
          @param maximum is the maximum value of the range (only used for Tsukamoto)
          @return 
         */
        virtual scalar defuzzify(const Term* term,
                scalar minimum, scalar maximum) const FL_IOVERRIDE;
        virtual WeightedSumCustom* clone() const FL_IOVERRIDE;

        static Defuzzifier* constructor();
    };
}

#endif  /* FL_WEIGHTEDSUMCUSTOM_H */

