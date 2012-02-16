// Copyright 2011 Yandex

#ifndef LTR_LEARNERS_GP_LEARNER_DETERMINANT_STRATEGY_H_
#define LTR_LEARNERS_GP_LEARNER_DETERMINANT_STRATEGY_H_

#include "ltr/learners/gp_learner.h"

#include <algorithm>

namespace ltr {
namespace gp {

template <typename TElement>
class GPLearnerWithDeterminantStrategy : public GPLearner<TElement> {
  public:
  /** Constructor creates a GPLearnerWithDeterminantStrategy.
   * \param p_Measure shared pointer to the measure that would be maximized on
   * a dataset within learning.
   * \param parameters the ParametersContainer parameters from which would
   * overwrite the default parameters.
   */
  GPLearnerWithDeterminantStrategy(typename Measure<TElement>::Ptr p_Measure,
      const ParametersContainer& parameters = ParametersContainer())
  : GPLearner<TElement>(p_Measure, parameters) {
    this->setDefaultParameters();
    this->parameters().copyParameters(parameters);
  }

  /** Constructor creates a GPLearnerWithDeterminantStrategy. But leaves
   * p_measure uninitialized.
   * \param parameters the ParametersContainer parameters from which would
   * overwrite the default parameters.
   */
  GPLearnerWithDeterminantStrategy(
      const ParametersContainer& parameters = ParametersContainer())
  : GPLearner<TElement>(parameters) {
    this->setDefaultParameters();
    this->parameters().copyParameters(parameters);
  }

  /** The function sets up default parameters for genetic learning process.
   */
  virtual void setDefaultParameters() {
    this->parameters().clear();
    this->parameters().setInt("POP_SIZE", 10);
    this->parameters().setInt("NBR_GEN", 3);
    this->parameters().setInt("MAX_DEPTH", 35);
    this->parameters().setInt("MIN_INIT_DEPTH", 2);
    this->parameters().setInt("MAX_INIT_DEPTH", 20);
    this->parameters().setDouble("INIT_GROW_PROBA", 0.5);
    this->parameters().setDouble("TOP_FOR_NEXT_GENERATION_PART", 0.3);
    this->parameters().setDouble("CROSSOVER_VS_MUTATION_PART", 0.7);
    this->parameters().setDouble("CROSSOVER_DISTRIB_PROBA", 0.3);
    this->parameters().setDouble("STANDART_MUTATION_VS_SWAP_MUTATION_PROBA",
        0.5);
    this->parameters().setInt("MUT_MAX_REGEN_DEPTH", 5);
    this->parameters().setDouble("MUT_SWAP_DISTRIB_PROBA", 0.5);
    this->parameters().setInt("SEED", 1);
    this->parameters().setBool("USE_ADD", true);
    this->parameters().setBool("USE_SUB", true);
    this->parameters().setBool("USE_MUL", true);
    this->parameters().setBool("USE_DIV", true);
    this->parameters().setBool("USE_IF", true);
    this->parameters().setBool("USE_EFEM", true);
  }
  /** The method checks the correctness of the parameters in the parameters
   * container. If one of them is not correct it throws
   * std::logical_error(PARAMETER_NAME).
   */
  virtual void checkParameters() const {
    checkIntParameterGreaterZero(*this, "POP_SIZE");
    checkIntParameterGreaterZero(*this, "NBR_GEN");
    checkIntParameterGreaterZero(*this, "MAX_DEPTH");
    checkIntParameterGreaterZero(*this, "MIN_INIT_DEPTH");
    checkIntParameterGreaterThan(*this, "MAX_INIT_DEPTH",
        this->parameters().getInt("MIN_INIT_DEPTH") - 1);
    checkDoubleParameterGreaterOrEqualZeroLessOrEqualOne(*this,
        "INIT_GROW_PROBA");
    checkDoubleParameterGreaterOrEqualZeroLessOrEqualOne(*this,
        "TOP_FOR_NEXT_GENERATION_PART");
    checkDoubleParameterGreaterOrEqualZeroLessOrEqualOne(*this,
        "CROSSOVER_VS_MUTATION_PART");
    checkDoubleParameterGreaterOrEqualZeroLessOrEqualOne(*this,
        "CROSSOVER_DISTRIB_PROBA");
    checkDoubleParameterGreaterOrEqualZeroLessOrEqualOne(*this,
        "STANDART_MUTATION_VS_SWAP_MUTATION_PROBA");
    checkIntParameterGreaterZero(*this, "MUT_MAX_REGEN_DEPTH");
    checkDoubleParameterGreaterOrEqualZeroLessOrEqualOne(*this,
        "MUT_SWAP_DISTRIB_PROBA");
  }

  private:
  /** \brief This function implements the changes made in the population at each
   *  algorithm's iteration.
   */
  virtual void evaluationStrategyStepImpl() {
    assert(this->population_.size() > 0);

    std::cout << "Population looks like:\n";
    for (size_t treeIdx = 0; treeIdx < this->population_.size(); ++treeIdx) {
      std::cout << this->population_.at(treeIdx).mFitness
          << "(" << this->population_.at(treeIdx).mValid <<") ";
    }
    std::cout << std::endl;

    std::cout << "Sorting population.\n";
    std::sort(this->population_.begin(), this->population_.end());

    size_t topBoundIdx =
        (1 - this->parameters().getDouble("TOP_FOR_NEXT_GENERATION_PART")) *
        this->population_.size();
    if (topBoundIdx == this->population_.size()) {
      topBoundIdx -= 1;
    }

    size_t curTreeFromTopIdx = topBoundIdx;
    for (size_t treeIdx = 0; treeIdx < topBoundIdx; ++treeIdx) {
      this->population_.at(treeIdx) = this->population_.at(curTreeFromTopIdx);
      ++curTreeFromTopIdx;
      if (curTreeFromTopIdx >= this->population_.size()) {
        curTreeFromTopIdx = topBoundIdx;
      }
    }

    size_t crossoverBoundIdx =
        this->parameters().getDouble("CROSSOVER_VS_MUTATION_PART") *
        topBoundIdx;
    if (crossoverBoundIdx % 2 != 1) {
      crossoverBoundIdx -= 1;
    }

    for (size_t treeIdx = 0; treeIdx < crossoverBoundIdx; treeIdx += 2) {
      Puppy::mateTrees(this->population_[treeIdx],
          this->population_[treeIdx + 1],
          this->context_,
          this->parameters().getDouble("CROSSOVER_DISTRIB_PROBA"),
          this->parameters().getInt("MAX_DEPTH"));
    }

    for (size_t treeIdx = crossoverBoundIdx; treeIdx < topBoundIdx; ++treeIdx) {
      if (this->context_.mRandom.rollUniform() <=
          this->parameters().getDouble(
              "STANDART_MUTATION_VS_SWAP_MUTATION_PROBA")) {
        Puppy::mutateStandard(this->population_[treeIdx],
            this->context_,
            this->parameters().getInt("MUT_MAX_REGEN_DEPTH"),
            this->parameters().getInt("MAX_DEPTH"));
      } else {
        Puppy::mutateSwap(this->population_[treeIdx],
            this->context_,
            this->parameters().getDouble("MUT_SWAP_DISTRIB_PROBA"));
      }
    }
  }
};
template class GPLearnerWithDeterminantStrategy<Object>;
template class GPLearnerWithDeterminantStrategy<ObjectPair>;
template class GPLearnerWithDeterminantStrategy<ObjectList>;
}
}
#endif  // LTR_LEARNERS_GP_LEARNER_DETERMINANT_STRATEGY_H_
