// Copyright 2012 Yandex

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "ltr/parameters_container/parameters_container.h"

#include "ltr/interfaces/parameterized.h"

using ltr::ParametersContainer;
using ltr::Parameterized;

TEST(InterfacesTest, ParameterizedTest) {
  ParametersContainer parameters_container;
  parameters_container.Set("bool1", false);
  parameters_container.Set("double1", 3.24);

  EXPECT_FALSE(parameters_container.Get<bool>("bool1"));
  EXPECT_EQ(3.24, parameters_container.Get<double>("double1"));

  EXPECT_ANY_THROW(parameters_container.Get<double>("none"));
};

