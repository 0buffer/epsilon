#ifndef POINCARE_DYNAMIC_HIERARCHY_H
#define POINCARE_DYNAMIC_HIERARCHY_H

#include <poincare/hierarchy.h>
#include <poincare/rational.h>

namespace Poincare {

class DynamicHierarchy : public Hierarchy {
public:
  DynamicHierarchy();
  DynamicHierarchy(const Expression * const * operands, int numberOfOperands, bool cloneOperands = true);
  DynamicHierarchy(const Expression * operand1, const Expression * operand2, bool cloneOperands = true) :
   DynamicHierarchy(ExpressionArray(operand1, operand2), 2, cloneOperands) {}
  ~DynamicHierarchy();
  DynamicHierarchy(const DynamicHierarchy & other) = delete;
  DynamicHierarchy(DynamicHierarchy && other) = delete;
  DynamicHierarchy& operator=(const DynamicHierarchy & other) = delete;
  DynamicHierarchy& operator=(DynamicHierarchy && other) = delete;

  int numberOfOperands() const override { return m_numberOfOperands; }
  const Expression * const * operands() const override { return m_operands; };

  void removeOperand(const Expression * e, bool deleteAfterRemoval = true);
  void addOperands(const Expression * const * operands, int numberOfOperands);
  void addOperandAtIndex(Expression * operand, int index);
  void mergeOperands(DynamicHierarchy * d);
  typedef int (*ExpressionOrder)(const Expression * e1, const Expression * e2);
  void sortOperands(ExpressionOrder order);
  Expression * squashUnaryHierarchy();
private:
  void removeOperandAtIndex(int i, bool deleteAfterRemoval);
  int simplificationOrderSameType(const Expression * e) const override;
  int simplificationOrderGreaterType(const Expression * e) const override;
  const Expression ** m_operands;
  int m_numberOfOperands;
};

}

#endif
