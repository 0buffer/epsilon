#ifndef POINCARE_EXPRESSION_H
#define POINCARE_EXPRESSION_H

#include <poincare/expression_layout.h>
extern "C" {
#include <assert.h>
}

namespace Poincare {

class Context;
template<class T>
class Evaluation;

class Expression {
  friend class Division;
  friend class Logarithm;
  friend class Opposite;
  friend class NaperianLogarithm;
  friend class Subtraction;
  friend class Addition;
  friend class Multiplication;
  friend class Power;
  friend class Trigonometry;
  friend class Tangent;
  friend class AbsoluteValue;
public:
  enum class Type : uint8_t {
    Undefined = 0,
    Rational = 1,
    Decimal,
    Multiplication,
    Power,
    Addition,
    Factorial,
    AbsoluteValue,
    ArcCosine,
    ArcSine,
    ArcTangent,
    BinomialCoefficient,
    Ceiling,
    ComplexArgument,
    ConfidenceInterval,
    Conjugate,
    Cosine,
    Derivative,
    Determinant,
    Division,
    DivisionQuotient,
    DivisionRemainder,
    Floor,
    FracPart,
    GreatCommonDivisor,
    HyperbolicArcCosine,
    HyperbolicArcSine,
    HyperbolicArcTangent,
    HyperbolicCosine,
    HyperbolicSine,
    HyperbolicTangent,
    ImaginaryPart,
    Integral,
    LeastCommonMultiple,
    Logarithm,
    MatrixDimension,
    MatrixInverse,
    MatrixTrace,
    MatrixTranspose,
    NaperianLogarithm,
    NthRoot,
    Opposite,
    Parenthesis,
    PermuteCoefficient,
    PredictionInterval,
    Product,
    RealPart,
    Round,
    Sine,
    SquareRoot,
    Store,
    Subtraction,
    Sum,
    Tangent,
    Symbol,

    Complex,
    ComplexMatrix,
    ExpressionMatrix,
    SimplificationRoot,
  };
  enum class FloatDisplayMode {
    Decimal = 0,
    Scientific = 1,
    Default = 2
  };
  enum class ComplexFormat {
    Cartesian = 0,
    Polar = 1,
    Default = 2
  };
  enum class AngleUnit {
    Degree = 0,
    Radian = 1,
    Default = 2
  };

  /* Constructor & Destructor */
  static Expression * parse(char const * string);
  virtual ~Expression() = default;
  virtual Expression * clone() const = 0;

  /* Poor man's RTTI */
  virtual Type type() const = 0;

  /* Circuit breaker */
  typedef bool (*CircuitBreaker)();
  static void setCircuitBreaker(CircuitBreaker cb);
  static bool shouldStopProcessing();

  /* Hierarchy */
  virtual const Expression * operand(int i) const = 0;
  Expression * editableOperand(int i) { return const_cast<Expression *>(operand(i)); }
  virtual int numberOfOperands() const = 0;
  Expression * parent() const { return m_parent; }
  void setParent(Expression * parent) { m_parent = parent; }
  bool hasAncestor(const Expression * e) const;
  virtual void replaceOperand(const Expression * oldOperand, Expression * newOperand, bool deleteOldOperand = true) = 0;
  Expression * replaceWith(Expression * newOperand, bool deleteAfterReplace = true);
  virtual void swapOperands(int i, int j) = 0;

  /* Properties */
  enum class Sign {
    Negative = -1,
    Unknown = 0,
    Positive = 1
  };
  virtual Sign sign() const { return Sign::Unknown; }
  typedef bool (*ExpressionTest)(const Expression * e);
  bool recursivelyMatches(ExpressionTest test) const;

  /* Comparison */
  /* isIdenticalTo is the "easy" equality, it returns true if both trees have
   * same structures and all their nodes have same types and values (ie,
   * sqrt(pi^2) is NOT identical to pi). */
  bool isIdenticalTo(const Expression * e) const {
    /* We use the simplification order only because it is a already-coded total
     * order on expresssions. */
    return SimplificationOrder(this, e) == 0;
  }

  /* Layout Engine */
  ExpressionLayout * createLayout(FloatDisplayMode floatDisplayMode = FloatDisplayMode::Default, ComplexFormat complexFormat = ComplexFormat::Default) const; // Returned object must be deleted
  virtual int writeTextInBuffer(char * buffer, int bufferSize) const = 0;

  /* Simplification */
  static void Simplify(Expression ** expressionAddress, Context & context, AngleUnit angleUnit = AngleUnit::Default);

  /* Evaluation Engine
   * The function evaluate creates a new expression and thus mallocs memory.
   * Do not forget to delete the new expression to avoid leaking. */
  template<typename T> Evaluation<T> * evaluate(Context& context, AngleUnit angleUnit = AngleUnit::Default) const;
  template<typename T> T approximate(Context& context, AngleUnit angleUnit = AngleUnit::Default) const;
  template<typename T> static T approximate(const char * text, Context& context, AngleUnit angleUnit = AngleUnit::Default);
protected:
  /* Constructor */
  Expression() : m_parent(nullptr) {}
  /* Evaluation Engine */
  typedef float SinglePrecision;
  typedef double DoublePrecision;
  template<typename T> static T epsilon();

  /* Simplification */
  /* SimplificationOrder returns:
   *   1 if e1 > e2
   *   -1 if e1 < e2
   *   0 if e1 == e
   * Following the order described in Computer Algebra and Symbolic Computation,
   * Joel S. Cohen (section 3.1). The order groups like terms together to avoid
   * quadratic complexity when factorizing addition or multiplication. For
   * example, it groups terms with same bases together (ie Pi, Pi^3)  and with
   * same non-rational factors together (ie Pi, 2*Pi). */
  static int SimplificationOrder(const Expression * e1, const Expression * e2);
private:
  /* Properties */
  virtual Expression * setSign(Sign s, Context & context, AngleUnit angleUnit) { assert(false); return nullptr; }
  /* Comparison */
  /* In the simplification order, most expressions are compared by only
   * comparing their types. However hierarchical expressions of same type would
   * compare their operands and thus need to reimplement
   * simplificationOrderSameType. Besides, operations that can be simplified
   * (ie +, *, ^, !) have specific rules to group like terms together and thus
   * reimplement simplificationOrderGreaterType. */
  virtual int simplificationOrderGreaterType(const Expression * e) const { return -1; }
  //TODO: What should be the implementation for complex?
  virtual int simplificationOrderSameType(const Expression * e) const { return 0; }
  /* Layout Engine */
  virtual ExpressionLayout * privateCreateLayout(FloatDisplayMode floatDisplayMode, ComplexFormat complexFormat) const = 0;
  /* Simplification */
  static void Reduce(Expression ** expressionAddress, Context & context, AngleUnit angleUnit);
  Expression * deepBeautify(Context & context, AngleUnit angleUnit);
  Expression * deepReduce(Context & context, AngleUnit angleUnit);
  // TODO: should be virtual pure
  virtual Expression * shallowReduce(Context & context, AngleUnit angleUnit) { return this; };
  virtual Expression * shallowBeautify(Context & context, AngleUnit angleUnit) { return this; };

  // Private methods used in simplification process
  virtual Expression * cloneDenominator(Context & context, AngleUnit angleUnit) const {
    return nullptr;
  }

  /* Evaluation Engine */
  virtual Evaluation<float> * privateEvaluate(SinglePrecision p, Context& context, AngleUnit angleUnit) const = 0;
  virtual Evaluation<double> * privateEvaluate(DoublePrecision p, Context& context, AngleUnit angleUnit) const = 0;

  Expression * m_parent;
};

}

#endif
