#include "cc/modules/common/tests/test.h"

#include "utils/eigen_util.h"

TEST_CASE("utils eigen_util", "[common][utils]") {
  MatrixXul MatUL(2, 3);
  MatrixXl MatL(20, 3);
  MatrixXui MatUI(2, 33);

  // print_shape
  print_shape(MatUL);
  print_shape(MatL);
  print_shape(MatUI);

  // reshape
  MatrixXul Mat1(20, 30);
  MatrixXul Mat2;
  print_shape(Mat1);
  reshape(Mat1, Mat2, 30, 20);
  print_shape(Mat2);
}