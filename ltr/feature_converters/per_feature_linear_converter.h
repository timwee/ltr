// Copyright 2011 Yandex

#ifndef LTR_FEATURE_CONVERTERS_PER_FEATURE_LINEAR_CONVERTER_H_
#define LTR_FEATURE_CONVERTERS_PER_FEATURE_LINEAR_CONVERTER_H_

#include <string>
#include <vector>

#include "ltr/feature_converters/feature_converter.h"

#include "ltr/utility/boost/shared_ptr.h"

using std::string;
using std::vector;

namespace ltr {
/**
 * \brief Perform linear transformation of each feature.
 *
 * features[i] ->  factors[i] * features[i] + shifts[i]
 * \param factors vector of factors
 * \param shifts vector of shifts
 */
class PerFeatureLinearConverter : public FeatureConverter {
  ALLOW_SHARED_PTR_ONLY_CREATION(PerFeatureLinearConverter)
 public:
  explicit PerFeatureLinearConverter(const ParametersContainer& parameters) {
  }

  explicit PerFeatureLinearConverter(
    const FeatureInfo& input_feature_info = FeatureInfo())
    : FeatureConverter(input_feature_info) {
      resize(input_feature_info);
  }

  virtual FeatureInfo convertFeatureInfo() const;
  /**
   * change expected feature info of input object and resizes
   * factors and shifts vectors
   */
  void resize(const FeatureInfo& input_feature_info);

  GET_SET_VECTOR(double, factor);
  GET_SET_VECTOR(double, shift);

  virtual string generateCppCode(const string& function_name) const;

  virtual string toString() const;
 private:
  virtual void applyImpl(const Object& input, Object* output) const;

  virtual string getDefaultAlias() const;

  vector<double> factor_;
  vector<double> shift_;
};
};
#endif  // LTR_FEATURE_CONVERTERS_PER_FEATURE_LINEAR_CONVERTER_H_
