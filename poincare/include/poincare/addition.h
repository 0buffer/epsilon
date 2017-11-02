#ifndef POINCARE_ADDITION_H
#define POINCARE_ADDITION_H

#include <poincare/dynamic_hierarchy.h>
#include <poincare/rational.h>
#include <poincare/layout_engine.h>
#include <poincare/evaluation_engine.h>

namespace Poincare {

class Addition : public DynamicHierarchy {
  using DynamicHierarchy::DynamicHierarchy;
  friend class Logarithm;
  friend class Multiplication;
  friend class Subtraction;
public:
  Type type() const override;
  Expression * clone() const override;
  /* Evaluation */
  template<typename T> static Complex<T> compute(const Complex<T> c, const Complex<T> d);
  template<typename T> static Evaluation<T> * computeOnMatrices(Evaluation<T> * m, Evaluation<T> * n) {
    return EvaluationEngine::elementWiseOnComplexMatrices(m, n, compute<T>);
  }
  template<typename T> static Evaluation<T> * computeOnComplexAndMatrix(const Complex<T> * c, Evaluation<T> * m) {
    return EvaluationEngine::elementWiseOnComplexAndComplexMatrix(c, m, compute<T>);
  }
private:
  /* Layout */
  ExpressionLayout * privateCreateLayout(FloatDisplayMode floatDisplayMode, ComplexFormat complexFormat) const override {
    return LayoutEngine::createInfixLayout(this, floatDisplayMode, complexFormat, name());
  }
  int writeTextInBuffer(char * buffer, int bufferSize) const override {
    return LayoutEngine::writeInfixExpressionTextInBuffer(this, buffer, bufferSize, name());
  }
  static const char * name() { return "+"; }

  /* Simplification */
  Expression * shallowSimplify(Context& context, AngleUnit angleUnit) override;
  Expression * shallowBeautify(Context & context, AngleUnit angleUnit) override;
  Expression * factorizeOnCommonDenominator(Context & context, AngleUnit angleUnit);
  void factorizeOperands(Expression * e1, Expression * e2, Context & context, AngleUnit angleUnit);
  static const Rational RationalFactor(Expression * e);
  static bool TermsHaveIdenticalNonRationalFactors(const Expression * e1, const Expression * e2);
  /* Evaluation */
  template<typename T> static Evaluation<T> * computeOnMatrixAndComplex(Evaluation<T> * m, const Complex<T> * c) {
    return EvaluationEngine::elementWiseOnComplexAndComplexMatrix(c, m, compute<T>);
  }
  virtual Evaluation<float> * privateEvaluate(SinglePrecision p, Context& context, AngleUnit angleUnit) const override {
    return EvaluationEngine::mapReduce<float>(this, context, angleUnit, compute<float>, computeOnComplexAndMatrix<float>, computeOnMatrixAndComplex<float>, computeOnMatrices<float>);
  }
  virtual Evaluation<double> * privateEvaluate(DoublePrecision p, Context& context, AngleUnit angleUnit) const override {
    return EvaluationEngine::mapReduce<double>(this, context, angleUnit, compute<double>, computeOnComplexAndMatrix<double>, computeOnMatrixAndComplex<double>, computeOnMatrices<double>);
  }
};

}

#endif
