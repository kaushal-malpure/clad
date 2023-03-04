//--------------------------------------------------------------------*- C++ -*-
// clad - The C++ Clang-based Automatic Differentiator
//
// Demo of a simple gradient decent algorithm to predict the feautres for a
// liner relationship. Clad is used to calculate the gradient of the
// cost function.
//
//----------------------------------------------------------------------------//

// To run the demo please type:
// path/to/clang++  -Xclang -add-plugin -Xclang clad -Xclang -load -Xclang \
// path/to/libclad.so  -I../include/ -x c++ -lstdc++ -lm GradientDescent.cpp
//
// A typical invocation would be:
// ../../../../obj/Debug+Asserts/bin/clang++  -Xclang -add-plugin -Xclang clad \
// -Xclang -load -Xclang ../../../../obj/Debug+Asserts/lib/libclad.dylib     \
// -I../include/ -x c++ -lstdc++ -lm GradientDescent.cpp
//
// To plot the results install gnuplot and type:
// gnuplot -e "plot 'dataset_gd.dat' with points pt 7; replot 'out_gd.dat' \
// using 1:2 with lines; pause -1"

#include <fstream>  // For plotting data.
#include <iostream> // For std::*
#include <vector>   // For std::vector.

// Necessary for clad to work include
#include "clad/Differentiator/Differentiator.h"

// The hypothesis function for the quadratic problem
// theta_0, theta_1 are the parameters to learn and x is the input data
double f(double theta_0, double theta_1, double x) {
  return theta_0 + theta_1 * x;
}

// Structure to represent input dataset
// x, y represents the input and output data respectively
// size represents the number of samples to be generated
struct Dataset {
  std::vector<double> x;
  std::vector<double> y;
  size_t size = 1000;
  double learning_rate = 1e-2;

  // Populate the data set with random data
  Dataset() {
    // For plotting data later
    std::ofstream out("dataset_gd.dat");

    // For storing the randomized values
    double rand_x, rand_y, t0, t1;

    for (size_t i = 0; i < size; i++) {
      // Randomly select normalized data.
      rand_x = 3 * (float)(std::rand() % 100) / 100;
      t0 = 9 + (float)(std::rand() % 100) / 100;
      t1 = 2;
      rand_y = f(t0, t1, rand_x);

      x.push_back(rand_x);
      y.push_back(rand_y);
      out << x[i] << "\t" << y[i] << std::endl;
    }

    out.close();
  }
};

// Function to perform a minimization step
// theta_x are the hypothesis parameters, t is the generated dataset and
// clad_grad is the gradient function generated by Clad
template <typename T>
void performStep(double& theta_0, double& theta_1, Dataset dt, T clad_grad) {
  double J_theta[4], result[2] = {0, 0};
  for (size_t i = 0; i < dt.size; i++) {
    J_theta[0] = J_theta[1] = J_theta[2] = J_theta[3] = 0;
    clad_grad.execute(theta_0, theta_1, dt.x[i], dt.y[i], &J_theta[0],
                      &J_theta[1], &J_theta[2], &J_theta[3]);

    result[0] += J_theta[0];
    result[1] += J_theta[1];
  }

  theta_0 -= dt.learning_rate * result[0] / (2 * dt.size);
  theta_1 -= dt.learning_rate * result[1] / (2 * dt.size);
}

// The cost function to minimize using gradient descent
// theta_x are the parameters to learn; x, y are the inputs and outputs of f
double cost(double theta_0, double theta_1, double x, double y) {
  double f_x = f(theta_0, theta_1, x);
  return (f_x - y) * (f_x - y);
}

// Function to optimize the cost function of interest
// theta is the hypothesis parameter list and maxSteps is the maximum steps to
// perform
std::vector<double> optimize(std::vector<double> theta, Dataset dt,
                             unsigned int maxSteps, double eps) {
  std::vector<double> diff = theta;
  bool hasConverged = false;
  int currentStep = 0;

  // Call for Clad to differentiate the cost function specified before
  auto clad_grad = clad::gradient(cost);

  do {
    performStep(theta[0], theta[1], dt, clad_grad);

    std::cout << "Steps #" << currentStep << " Theta 0: " << theta[0]
              << " Theta 1: " << theta[1] << std::endl;

    hasConverged = abs(diff[0] - theta[0]) <= eps &&
                   abs(diff[1] - theta[1]) <= eps;

    diff = theta;
  } while (currentStep++ < maxSteps && !hasConverged);

  return theta;
}

int main() {
  std::vector<double> theta = {0, 0};
  Dataset dt;
  auto results = optimize(theta, dt, 10000, 1e-6);

  std::cout << "Result: "
            << "(" << results[0] << ", " << results[1] << ")" << std::endl;

  // For plotting data
  std::ofstream out("out_gd.dat");
  for (size_t i = 0; i < dt.size; i++) {
    out << dt.x[i] << "\t" << f(results[0], results[1], dt.x[i]) << std::endl;
  }
  out.close();

  return 0;
}
