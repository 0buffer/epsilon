extern "C" {
#include <assert.h>
#include <stdlib.h>
}
#include <cmath>
#include <math.h>
#include <ion.h>
#include <poincare/matrix_inverse.h>
#include <poincare/matrix.h>
#include <poincare/power.h>
#include <poincare/opposite.h>
#include <poincare/parenthesis.h>
#include <poincare/addition.h>
#include <poincare/undefined.h>
#include <poincare/square_root.h>
#include <poincare/nth_root.h>
#include <poincare/division.h>
#include <poincare/matrix_inverse.h>
#include <poincare/arithmetic.h>
#include <poincare/symbol.h>
#include <poincare/subtraction.h>
#include <poincare/cosine.h>
#include <poincare/sine.h>
#include "layout/baseline_relative_layout.h"

namespace Poincare {

Expression::Type Power::type() const {
  return Type::Power;
}

Expression * Power::clone() const {
  return new Power(m_operands, true);
}

Expression::Sign Power::sign() const {
  if (operand(0)->sign() == Sign::Positive && operand(1)->sign() != Sign::Unknown) {
    return Sign::Positive;
  }
  if (operand(0)->sign() == Sign::Negative && operand(1)->type() == Type::Rational) {
    const Rational * r = static_cast<const Rational *>(operand(1));
    if (r->denominator().isOne()) {
      if (Integer::Division(r->numerator(), Integer(2)).remainder.isZero()) {
        return Sign::Positive;
      } else {
        return Sign::Negative;
      }
    }
  }
  return Sign::Unknown;
}

Expression * Power::setSign(Sign s, Context & context, AngleUnit angleUnit) {
  assert(s == Sign::Positive);
  assert(operand(0)->sign() == Sign::Negative);
  editableOperand(0)->setSign(Sign::Positive, context, angleUnit);
  return this;
}

template<typename T>
Complex<T> Power::compute(const Complex<T> c, const Complex<T> d) {
  if (d.b() != 0) {
    /* First case c and d is complex */
    if (c.b() != 0 || c.a() <= 0) {
      return Complex<T>::Float(NAN);
    }
    /* Second case only d is complex */
    T radius = std::pow(c.a(), d.a());
    T theta = d.b()*std::log(c.a());
    return Complex<T>::Polar(radius, theta);
  }
  /* Third case only c is complex */
  T radius = std::pow(c.r(), d.a());
  T theta = d.a()*c.th();
  return Complex<T>::Polar(radius, theta);
}

template<typename T> Matrix * Power::computeOnMatrixAndComplex(const Matrix * m, const Complex<T> * d) {
 if (m->numberOfRows() != m->numberOfColumns()) {
    return nullptr;
  }
  T power = d->toScalar();
  if (isnan(power) || isinf(power) || power != (int)power || std::fabs(power) > k_maxApproximatePowerMatrix) {
    return nullptr;
  }
  if (power < 0) {
    Matrix * inverse = m->createInverse<T>();
    if (inverse == nullptr) {
      return nullptr;
    }
    Complex<T> minusC = Opposite::compute(*d, AngleUnit::Default);
    Matrix * result = Power::computeOnMatrixAndComplex(inverse, &minusC);
    delete inverse;
    return result;
  }
  Matrix * result = Matrix::createApproximateIdentity<T>(m->numberOfRows());
  // TODO: implement a quick exponentiation
  for (int k = 0; k < (int)power; k++) {
    if (shouldStopProcessing()) {
      delete result;
      return nullptr;
    }
    result = Multiplication::computeOnMatrices<T>(result, m);
  }
  return result;
}

ExpressionLayout * Power::privateCreateLayout(FloatDisplayMode floatDisplayMode, ComplexFormat complexFormat) const {
  assert(floatDisplayMode != FloatDisplayMode::Default);
  assert(complexFormat != ComplexFormat::Default);
  const Expression * indiceOperand = m_operands[1];
  // Delete eventual parentheses of the indice in the pretty print
  if (m_operands[1]->type() == Type::Parenthesis) {
    indiceOperand = m_operands[1]->operand(0);
  }
  return new BaselineRelativeLayout(m_operands[0]->createLayout(floatDisplayMode, complexFormat),indiceOperand->createLayout(floatDisplayMode, complexFormat), BaselineRelativeLayout::Type::Superscript);
}

int Power::simplificationOrderSameType(const Expression * e) const {
  int baseComparison = SimplificationOrder(operand(0), e->operand(0));
  if (baseComparison != 0) {
    return baseComparison;
  }
  return SimplificationOrder(operand(1), e->operand(1));
}

int Power::simplificationOrderGreaterType(const Expression * e) const {
  int baseComparison = SimplificationOrder(operand(0), e);
  if (baseComparison != 0) {
    return baseComparison;
  }
  Rational one(1);
  return SimplificationOrder(operand(1), &one);
}

Expression * Power::shallowReduce(Context& context, AngleUnit angleUnit) {
  Expression * e = Expression::shallowReduce(context, angleUnit);
  if (e != this) {
    return e;
  }
#if MATRIX_EXACT_REDUCING
  /* Step 0: get rid of matrix */
  if (operand(1)->type() == Type::Matrix) {
    return replaceWith(new Undefined(), true);
  }
  if (operand(0)->type() == Type::Matrix) {
    Matrix * mat = static_cast<Matrix *>(editableOperand(0));
    if (operand(1)->type() != Type::Rational || !static_cast<const Rational *>(operand(1))->denominator().isOne()) {
      return replaceWith(new Undefined(), true);
    }
    Integer exponent = static_cast<const Rational *>(operand(1))->numerator();
    if (mat->numberOfRows() != mat->numberOfColumns()) {
      return replaceWith(new Undefined(), true);
    }
    if (exponent.isNegative()) {
      editableOperand(1)->setSign(Sign::Positive, context, angleUnit);
      Expression * newMatrix = shallowReduce(context, angleUnit);
      Expression * parent = newMatrix->parent();
      MatrixInverse * inv = new MatrixInverse(newMatrix, false);
      parent->replaceOperand(newMatrix, inv, false);
      return inv;
    }
    if (Integer::NaturalOrder(exponent, Integer(k_maxExactPowerMatrix)) > 0) {
      return this;
    }
    int exp = exponent.extractedInt(); // Ok, because 0 < exponent < k_maxExactPowerMatrix
    Matrix * id = Matrix::createIdentity(mat->numberOfRows());
    if (exp == 0) {
      return replaceWith(id, true);
    }
    Multiplication * result = new Multiplication(id, mat->clone());
    // TODO: implement a quick exponentiation
    for (int k = 1; k < exp; k++) {
      result->addOperand(mat->clone());
    }
    replaceWith(result, true);
    return result->shallowReduce(context, angleUnit);
  }
#endif

  /* Step 0: We look for square root and sum of square roots (two terms maximum
   * so far) at the denominator and move them to the numerator. */
  Expression * r = removeSquareRootsFromDenominator(context, angleUnit);
  if (r) {
    return r;
  }

  if (operand(1)->type() == Type::Rational) {
    const Rational * b = static_cast<const Rational *>(operand(1));
    // x^0
    if (b->isZero()) {
      return replaceWith(new Rational(1), true);
    }
    // x^1
    if (b->isOne()) {
      return replaceWith(editableOperand(0), true);
    }
    // i^(p/q)
    if (operand(0)->type() == Type::Symbol && static_cast<const Symbol *>(operand(0))->name() == Ion::Charset::IComplex) {
      Rational r = Rational::Multiplication(*b, Rational(1, 2));
      return replaceWith(CreateNthRootOfUnity(r))->shallowReduce(context, angleUnit);
    }
  }
  if (operand(0)->type() == Type::Rational) {
    Rational * a = static_cast<Rational *>(editableOperand(0));
    // 0^x
    if (a->isZero()) {
      if (operand(1)->sign() == Sign::Positive) {
        return replaceWith(new Rational(0), true);
      }
      if (operand(1)->sign() == Sign::Negative) {
        return replaceWith(new Undefined(), true);
      }
    }
    // 1^x
    if (a->isOne()) {
      return replaceWith(new Rational(1), true);
    }
    // p^q with p, q rationals
    if (operand(1)->type() == Type::Rational) {
      return simplifyRationalRationalPower(this, a, static_cast<Rational *>(editableOperand(1)), context, angleUnit);
    }
  }
  // e^(i*Pi*r) with r rational
  if (isNthRootOfUnity()) {
    Expression * m = editableOperand(1);
    detachOperand(m);
    Expression * i = m->editableOperand(m->numberOfOperands()-1);
    static_cast<Multiplication *>(m)->removeOperand(i, false);
    if (angleUnit == AngleUnit::Degree) {
      const Expression * pi = m->operand(m->numberOfOperands()-1);
      m->replaceOperand(pi, new Rational(180), true);
    }
    Expression * cos = new Cosine(m, false);
    m = m->shallowReduce(context, angleUnit);
    Expression * sin = new Sine(m, true);
    Expression * complexPart = new Multiplication(sin, i, false);
    sin->shallowReduce(context, angleUnit);
    Expression * a = new Addition(cos, complexPart, false);
    cos->shallowReduce(context, angleUnit);
    complexPart->shallowReduce(context, angleUnit);
    return replaceWith(a, true)->shallowReduce(context, angleUnit);
  }
  // x^log(y,x)->y if y > 0
  if (operand(1)->type() == Type::Logarithm) {
    if (operand(1)->numberOfOperands() == 2 && operand(0)->isIdenticalTo(operand(1)->operand(1))) {
      // y > 0
      if (operand(1)->operand(0)->sign() == Sign::Positive) {
        return replaceWith(editableOperand(1)->editableOperand(0), true);
      }
    }
    // 10^log(y)
    if (operand(1)->numberOfOperands() == 1 && operand(0)->type() == Type::Rational && static_cast<const Rational *>(operand(0))->isTen()) {
      return replaceWith(editableOperand(1)->editableOperand(0), true);
    }
  }
  // (a^b)^c -> a^(b+c) if a > 0 or c is integer
  if (operand(0)->type() == Type::Power) {
    Power * p = static_cast<Power *>(editableOperand(0));
    // Check is a > 0 or c is Integer
    if (p->operand(0)->sign() == Sign::Positive ||
        (operand(1)->type() == Type::Rational && static_cast<Rational *>(editableOperand(1))->denominator().isOne())) {
      return simplifyPowerPower(p, editableOperand(1), context, angleUnit);
    }
  }
  // (a*b*c*...)^r ?
  if (operand(0)->type() == Type::Multiplication) {
    Multiplication * m = static_cast<Multiplication *>(editableOperand(0));
    // (a*b*c*...)^n = a^n*b^n*c^n*... if n integer
    if (operand(1)->type() == Type::Rational && static_cast<Rational *>(editableOperand(1))->denominator().isOne()) {
      return simplifyPowerMultiplication(m, editableOperand(1), context, angleUnit);
    }
    // (a*b*...)^r -> |a|^r*(sign(a)*b*)^r if a rational
    for (int i = 0; i < m->numberOfOperands(); i++) {
      if (m->operand(i)->sign() == Sign::Positive || m->operand(i)->type() == Type::Rational) {
        Expression * r = editableOperand(1);
        Expression * rCopy = r->clone();
        Expression * factor = m->editableOperand(i);
        if (factor->sign() == Sign::Negative) {
          m->replaceOperand(factor, new Rational(-1), false);
          factor->setSign(Sign::Positive, context, angleUnit);
        } else {
          m->removeOperand(factor, false);
        }
        m->shallowReduce(context, angleUnit);
        Power * p = new Power(factor, rCopy, false);
        Multiplication * root = new Multiplication(p, clone(), false);
        p->shallowReduce(context, angleUnit);
        root->editableOperand(1)->shallowReduce(context, angleUnit);
        replaceWith(root, true);
        return root->shallowReduce(context, angleUnit);
      }
    }
  }
  // a^(b+c) -> Rational(a^b)*a^c with a and b rational
  if (operand(0)->type() == Type::Rational && operand(1)->type() == Type::Addition) {
    Addition * a = static_cast<Addition *>(editableOperand(1));
    // Check is b is rational
    if (a->operand(0)->type() == Type::Rational) {
      Power * p1 = static_cast<Power *>(clone());
      replaceOperand(a, a->editableOperand(1), true);
      Power * p2 = static_cast<Power *>(clone());
      Multiplication * m = new Multiplication(p1, p2, false);
      simplifyRationalRationalPower(p1, static_cast<Rational *>(p1->editableOperand(0)), static_cast<Rational *>(p1->editableOperand(1)->editableOperand(0)), context, angleUnit);
      replaceWith(m, true);
      return m->shallowReduce(context, angleUnit);
    }
  }
  return this;
}

Expression * Power::simplifyPowerPower(Power * p, Expression * e, Context& context, AngleUnit angleUnit) {
  Expression * p0 = p->editableOperand(0);
  Expression * p1 = p->editableOperand(1);
  p->detachOperands();
  Multiplication * m = new Multiplication(p1, e, false);
  replaceOperand(e, m, false);
  replaceOperand(p, p0, true);
  m->shallowReduce(context, angleUnit);
  return shallowReduce(context, angleUnit);
}

Expression * Power::simplifyPowerMultiplication(Multiplication * m, Expression * r, Context& context, AngleUnit angleUnit) {
  for (int index = 0; index < m->numberOfOperands(); index++) {
    Expression * factor = m->editableOperand(index);
    Power * p = new Power(factor, r, true); // We copy r and factor to avoid inheritance issues
    m->replaceOperand(factor, p, true);
    p->shallowReduce(context, angleUnit);
  }
  detachOperand(m);
  return replaceWith(m, true)->shallowReduce(context, angleUnit); // delete r
}

Expression * Power::simplifyRationalRationalPower(Expression * result, Rational * a, Rational * b, Context& context, AngleUnit angleUnit) {
  if (b->denominator().isOne()) {
    Rational r = Rational::Power(*a, b->numerator());
    return result->replaceWith(new Rational(r),true);
  }
  Expression * n = nullptr;
  Expression * d = nullptr;
  if (b->sign() == Sign::Negative) {
    b->setSign(Sign::Positive);
    n = CreateSimplifiedIntegerRationalPower(a->denominator(), b, false, context, angleUnit);
    d = CreateSimplifiedIntegerRationalPower(a->numerator(), b, true, context, angleUnit);
  } else {
    n = CreateSimplifiedIntegerRationalPower(a->numerator(), b, false, context, angleUnit);
    d = CreateSimplifiedIntegerRationalPower(a->denominator(), b, true, context, angleUnit);
  }
  Multiplication * m = new Multiplication(n, d, false);
  result->replaceWith(m, true);
  return m->shallowReduce(context, angleUnit);
}

Expression * Power::CreateSimplifiedIntegerRationalPower(Integer i, Rational * r, bool isDenominator, Context & context, AngleUnit angleUnit) {
  assert(!i.isZero());
  assert(r->sign() == Sign::Positive);
  if (i.isOne()) {
    return new Rational(1);
  }
  if (Arithmetic::k_primorial32.isLowerThan(i)) {
    r->setSign(isDenominator ? Sign::Negative : Sign::Positive);
    // We do not want to break i in prime factor because it might be take too many factors... More than k_maxNumberOfPrimeFactors.
    return new Power(new Rational(i), r->clone(), false);
  }
  Integer factors[Arithmetic::k_maxNumberOfPrimeFactors];
  Integer coefficients[Arithmetic::k_maxNumberOfPrimeFactors];
  Arithmetic::PrimeFactorization(&i, factors, coefficients, Arithmetic::k_maxNumberOfPrimeFactors);

  Integer r1(1);
  Integer r2(1);
  int index = 0;
  while (!coefficients[index].isZero() && index < Arithmetic::k_maxNumberOfPrimeFactors) {
    Integer n = Integer::Multiplication(coefficients[index], r->numerator());
    IntegerDivision div = Integer::Division(n, r->denominator());
    r1 = Integer::Multiplication(r1, Integer::Power(factors[index], div.quotient));
    r2 = Integer::Multiplication(r2, Integer::Power(factors[index], div.remainder));
    index++;
  }
  Rational * p1 = new Rational(r2);
  Integer one = isDenominator ? Integer(-1) : Integer(1);
  Rational * p2 = new Rational(one, r->denominator());
  Power * p = new Power(p1, p2, false);
  if (r1.isEqualTo(Integer(1)) && !i.isNegative()) {
    return p;
  }
  Rational * r3 = isDenominator ? new Rational(Integer(1), r1) : new Rational(r1);
  Multiplication * m = new Multiplication(r3, p, false);
  if (r2.isOne()) {
    m->removeOperand(p);
  }
  if (i.isNegative()) {
    Expression * nthRootOfUnity = CreateNthRootOfUnity(*r);
    m->addOperand(nthRootOfUnity);
    nthRootOfUnity->shallowReduce(context, angleUnit);

  }
  m->sortOperands(SimplificationOrder);
  return m;
}

Expression * Power::CreateNthRootOfUnity(const Rational r) {
  const Symbol * exp = new Symbol(Ion::Charset::Exponential);
  const Symbol * iComplex = new Symbol(Ion::Charset::IComplex);
  const Symbol * pi = new Symbol(Ion::Charset::SmallPi);
  const Expression * multExpOperands[3] = {iComplex, pi, new Rational(r)};
  Multiplication * mExp = new Multiplication(multExpOperands, 3, false);
  mExp->sortOperands(SimplificationOrder);
  return new Power(exp, mExp, false);
#if 0
  const Symbol * iComplex = new Symbol(Ion::Charset::IComplex);
  const Symbol * pi = new Symbol(Ion::Charset::SmallPi);
  Expression * op = new Multiplication(pi, r->clone(), false);
  Cosine * cos = new Cosine(op, false);
  op = op->shallowReduce(context, angleUnit);
  Sine * sin = new Sine(op, true);
  Expression * m = new Multiplication(iComplex, sin, false);
  sin->shallowReduce(context, angleUnit);
  Expression * a = new Addition(cos, m, false);
  cos->shallowReduce(context, angleUnit);
  const Expression * multExpOperands[3] = {pi, r->clone()};
#endif
}

Expression * Power::shallowBeautify(Context& context, AngleUnit angleUnit) {
  // X^-y -> 1/(X->shallowBeautify)^y
  if (operand(1)->sign() == Sign::Negative) {
    Expression * p = cloneDenominator(context, angleUnit);
    Division * d = new Division(new Rational(1), p, false);
    p->shallowReduce(context, angleUnit);
    replaceWith(d, true);
    return d->shallowBeautify(context, angleUnit);
  }
  if (operand(1)->type() == Type::Rational && static_cast<const Rational *>(operand(1))->numerator().isOne()) {
    Integer index = static_cast<const Rational *>(operand(1))->denominator();
    if (index.isEqualTo(Integer(2))) {
      const Expression * sqrtOperand[1] = {operand(0)};
      SquareRoot * sqr = new SquareRoot(sqrtOperand, true);
      return replaceWith(sqr, true);
    }
    const Expression * rootOperand[2] = {operand(0)->clone(), new Rational(index)};
    NthRoot * nr = new NthRoot(rootOperand, false);
    return replaceWith(nr, true);
  }
  // +(a,b)^c ->(+(a,b))^c
  if (operand(0)->type() == Type::Addition || operand(0)->type() == Type::Multiplication) {
    const Expression * o[1] = {operand(0)};
    Parenthesis * p = new Parenthesis(o, true);
    replaceOperand(operand(0), p, true);
  }
  return this;
}

Expression * Power::cloneDenominator(Context & context, AngleUnit angleUnit) const {
  if (operand(1)->sign() == Sign::Negative) {
    Expression * denominator = clone();
    Expression * newExponent = denominator->editableOperand(1)->setSign(Sign::Positive, context, angleUnit);
    if (newExponent->type() == Type::Rational && static_cast<Rational *>(newExponent)->isOne()) {
      delete denominator;
      return operand(0)->clone();
    }
    return denominator;
  }
  return nullptr;
}

Expression * Power::removeSquareRootsFromDenominator(Context & context, AngleUnit angleUnit) {
  Expression * result = nullptr;

  if (operand(0)->type() == Type::Rational && operand(1)->type() == Type::Rational && (static_cast<const Rational *>(operand(1))->isHalf() || static_cast<const Rational *>(operand(1))->isMinusHalf())) {
      /* We're considering a term of the form sqrt(p/q) (or 1/sqrt(p/q)), with
       * p and q integers.
       * We'll turn those into sqrt(p*q)/q (or sqrt(p*q)/p) . */
      Integer p = static_cast<const Rational *>(operand(0))->numerator();
      Integer q = static_cast<const Rational *>(operand(0))->denominator();
      // We do nothing for terms of the form sqrt(p)
      if (!q.isOne() || static_cast<const Rational *>(operand(1))->isMinusHalf()) {
        Power * sqrt = new Power(new Rational(Integer::Multiplication(p, q)), new Rational(1, 2), false);
        if (static_cast<const Rational *>(operand(1))->isHalf()) {
          result = new Multiplication(new Rational(Integer(1), q), sqrt, false);
        } else {
          result = new Multiplication(new Rational(Integer(1), p), sqrt, false);
        }
        sqrt->shallowReduce(context, angleUnit);
      }
  } else if (operand(1)->type() == Type::Rational && static_cast<const Rational *>(operand(1))->isMinusOne() && operand(0)->type() == Type::Addition && operand(0)->numberOfOperands() == 2 && TermIsARationalSquareRootOrRational(operand(0)->operand(0)) && TermIsARationalSquareRootOrRational(operand(0)->operand(1))) {
    /* We're considering a term of the form
     *
     * 1/(n1/d1*sqrt(p1/q1) + n2/d2*sqrt(p2/q2))
     *
     * and we want to turn it into
     *
     *  n1*q2*d1*d2^2*sqrt(p1*q1) - n2*q1*d2*d1^2*sqrt(p2*q2)
     * -------------------------------------------------------
     *          n1^2*d2^2*p1*q2 - n2^2*d1^2*p2*q1
     */
    const Rational * f1 = RationalFactorInExpression(operand(0)->operand(0));
    const Rational * f2 = RationalFactorInExpression(operand(0)->operand(1));
    const Rational * r1 = RadicandInExpression(operand(0)->operand(0));
    const Rational * r2 = RadicandInExpression(operand(0)->operand(1));
    Integer n1 = (f1 ? f1->numerator() : Integer(1));
    Integer d1 = (f1 ? f1->denominator() : Integer(1));
    Integer p1 = (r1 ? r1->numerator() : Integer(1));
    Integer q1 = (r1 ? r1->denominator() : Integer(1));
    Integer n2 = (f2 ? f2->numerator() : Integer(1));
    Integer d2 = (f2 ? f2->denominator() : Integer(1));
    Integer p2 = (r2 ? r2->numerator() : Integer(1));
    Integer q2 = (r2 ? r2->denominator() : Integer(1));

    // Compute the denominator = n1^2*d2^2*p1*q2 - n2^2*d1^2*p2*q1
    Integer denominator = Integer::Subtraction(
        Integer::Multiplication(
          Integer::Multiplication(
            Integer::Power(n1, Integer(2)),
            Integer::Power(d2, Integer(2))),
          Integer::Multiplication(p1, q2)),
        Integer::Multiplication(
          Integer::Multiplication(
            Integer::Power(n2, Integer(2)),
            Integer::Power(d1, Integer(2))),
          Integer::Multiplication(p2, q1)));

    // Compute the numerator
    Power * sqrt1 = new Power(new Rational(Integer::Multiplication(p1, q1)), new Rational(1, 2), false);
    Power * sqrt2 = new Power(new Rational(Integer::Multiplication(p2, q2)), new Rational(1, 2), false);
    Integer factor1 = Integer::Multiplication(
        Integer::Multiplication(n1, d1),
        Integer::Multiplication(Integer::Power(d2, Integer(2)), q2));
    Multiplication * m1 = new Multiplication(new Rational(factor1), sqrt1, false);
    Integer factor2 = Integer::Multiplication(
        Integer::Multiplication(n2, d2),
        Integer::Multiplication(Integer::Power(d1, Integer(2)), q1));
    Multiplication * m2 = new Multiplication(new Rational(factor2), sqrt2, false);
    Subtraction * numerator = nullptr;
    if (denominator.isNegative()) {
      numerator = new Subtraction(m2, m1, false);
      denominator.setNegative(false);
    } else {
      numerator = new Subtraction(m1, m2, false);
    }

    result = new Multiplication(numerator, new Rational(Integer(1), denominator), false);
    numerator->deepReduce(context, angleUnit);
  }

  if (result) {
    replaceWith(result, true);
    result = result->shallowReduce(context, angleUnit);
  }
  return result;
}

bool Power::isNthRootOfUnity() const {
  if (operand(0)->type() != Type::Symbol || static_cast<const Symbol *>(operand(0))->name() != Ion::Charset::Exponential) {
    return false;
  }
  if (operand(1)->type() != Type::Multiplication) {
    return false;
  }
  if (operand(1)->numberOfOperands() < 2 || operand(1)->numberOfOperands() > 3) {
    return false;
  }
  const Expression * i = operand(1)->operand(operand(1)->numberOfOperands()-1);
  if (i->type() != Type::Symbol || static_cast<const Symbol *>(i)->name() != Ion::Charset::IComplex) {
    return false;
  }
  const Expression * pi = operand(1)->operand(operand(1)->numberOfOperands()-2);
  if (pi->type() != Type::Symbol || static_cast<const Symbol *>(pi)->name() != Ion::Charset::SmallPi) {
    return false;
  }
  if (numberOfOperands() == 2) {
    return true;
  }
  if (operand(1)->operand(0)->type() == Type::Rational) {
    return true;
  }
  return false;
}

}
