// Copyright 2011 Yandex

#ifndef LTR_FEATURE_CONVERTERS_FEATURE_NORMALIZER_LEARNER_H_
#define LTR_FEATURE_CONVERTERS_FEATURE_NORMALIZER_LEARNER_H_

#include <boost/shared_ptr.hpp>

#include <vector>
#include <algorithm>
#include <limits>
#include <string>
#include <sstream>

#include "ltr/feature_converters/feature_converter_learner.h"
#include "ltr/feature_converters/per_feature_linear_converter.h"
#include "ltr/utility/numerical.h"
#include "ltr/data/utility/data_set_statistics.h"

using std::vector;
using std::string;
using std::logic_error;

namespace ltr {

template <class TElement>
class FeatureNormalizerLearner
    : public FeatureConverterLearner<TElement, PerFeatureLinearConverter> {
 public:
  typedef boost::shared_ptr<FeatureNormalizerLearner> Ptr;

  explicit FeatureNormalizerLearner(const ParametersContainer& parameters =
      ParametersContainer()) {
    this->setDefaultParameters();
    this->copyParameters(parameters);
    this->checkParameters();
  }

  void learn(const DataSet<TElement>& data_set);
  PerFeatureLinearConverter::Ptr makeSpecific() const;

  void setDefaultParameters();
  void checkParameters() const;

  string toString() const;
 private:
  size_t featureCount() const;

  PerFeatureLinearConverter::Ptr converter_;
  vector<double> feature_min_statistic_;
  vector<double> feature_max_statistic_;
};

template <typename TElement>
void
FeatureNormalizerLearner<TElement>::learn(const DataSet<TElement>& data_set) {
  converter_ = PerFeatureLinearConverter::Ptr(new PerFeatureLinearConverter);
  utility::calcMinMaxStatistics(data_set, &feature_min_statistic_,
      &feature_max_statistic_);
  converter_->resize(this->featureCount());
  double normalizationIntervalBegin =
      this->parameters().template Get<double>("NormalizationIntervalBegin");
  double normalizationIntervalEnd =
      this->parameters().template Get<double>("NormalizationIntervalEnd");

  for (size_t feature_idx = 0;
      feature_idx < this->featureCount();
      ++feature_idx) {
    double delta = feature_max_statistic_[feature_idx]
                                          - feature_min_statistic_[feature_idx];
    double coefficient;
    double shift;
    if (utility::DoubleEqual(delta, 0)) {
      coefficient = 0.0;
      shift = (normalizationIntervalEnd - normalizationIntervalBegin) / 2;
    } else {
      coefficient =
          (normalizationIntervalEnd - normalizationIntervalBegin) / delta;
      shift = normalizationIntervalBegin -
          coefficient * feature_min_statistic_[feature_idx];
    }
    converter_->setCoefficient(feature_idx, coefficient);
    converter_->setShift(feature_idx, shift);
  }
  converter_->set_input_feature_info(data_set.featureInfo());
}

template <typename TElement>
string FeatureNormalizerLearner<TElement>::toString() const {
  std::stringstream str;
  std::fixed(str);
  str.precision(2);
  str << "Feature normalizer feature converter learner with parameters:";
  str << " NormalizationIntervalBegin = ";
  str << this->parameters().template Get<double>("NormalizationIntervalBegin");
  str << ", NormalizationIntervalEnd = ";
  str << this->parameters().template Get<double>("NormalizationIntervalEnd");
  return str.str();
}

template <typename TElement>
size_t FeatureNormalizerLearner<TElement>::featureCount() const {
  return feature_min_statistic_.size();
}

template <typename TElement>
PerFeatureLinearConverter::Ptr FeatureNormalizerLearner<TElement>::makeSpecific() const {
  return converter_;
}

template <typename TElement>
void FeatureNormalizerLearner<TElement>::setDefaultParameters() {
  this->clearParameters();
  this->addNewParam("NormalizationIntervalBegin", 0.0);
  this->addNewParam("NormalizationIntervalEnd", 1.0);
}

template <typename TElement>
void FeatureNormalizerLearner<TElement>::checkParameters() const {
  const ParametersContainer &params = this->parameters();
  if (params.Get<double>("NormalizationIntervalBegin") >=
      params.Get<double>("NormalizationIntervalEnd")) {
    throw logic_error("Bad parameters for FeatureNormalizerLearner");
  }
}
}
#endif  // LTR_FEATURE_CONVERTERS_FEATURE_NORMALIZER_LEARNER_H_
