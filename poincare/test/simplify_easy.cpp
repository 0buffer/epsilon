#include <quiz.h>
#include <poincare.h>
#include <ion.h>
#include <assert.h>
#include "helper.h"
#if POINCARE_TESTS_PRINT_EXPRESSIONS
#include "../src/expression_debug.h"
#include <iostream>
using namespace std;
#endif

using namespace Poincare;

void assert_parsed_expression_simplify_to(const char * expression, const char * simplifiedExpression) {
  Expression * e = parse_expression(expression);
#if POINCARE_TESTS_PRINT_EXPRESSIONS
  cout << "---- Simplify: " << expression << "----"  << endl;
#endif
  Expression::simplifyAndBeautify(&e);
#if POINCARE_TESTS_PRINT_EXPRESSIONS
  print_expression(e, 0);
#endif
  Expression * f = parse_expression(simplifiedExpression);
#if POINCARE_TESTS_PRINT_EXPRESSIONS
  cout << "---- compared to: " << simplifiedExpression << "----"  << endl;
  Expression::simplifyAndBeautify(&f);
  print_expression(f, 0);
#endif
  assert(e->compareTo(f) == 0);
  delete e;
  delete f;
}


QUIZ_CASE(poincare_simplify_easy) {
  assert_parsed_expression_simplify_to("1+1+ln(2)+(5+3*2)/9-4/7+1/98", "ln(2)+2347/882");
  assert_parsed_expression_simplify_to("3/4+5/4-12+1/567", "-5669/567");
  assert_parsed_expression_simplify_to("34/78+67^(-1)", "1178/2613");
  assert_parsed_expression_simplify_to("root(4,5)", "4^(1/5)");
  assert_parsed_expression_simplify_to("R(4)", "2");
  assert_parsed_expression_simplify_to("3^4", "81");
  assert_parsed_expression_simplify_to("3^(-4)", "1/81");
  assert_parsed_expression_simplify_to("12348/34564", "3087/8641");
  assert_parsed_expression_simplify_to("1256^(1/3)*x", "2*157^(1/3)*x");
  assert_parsed_expression_simplify_to("1256^(-1/3)", "2^(-1)*157^(-1/3)");
  assert_parsed_expression_simplify_to("32^(-1/5)", "1/2");
  assert_parsed_expression_simplify_to("ln(2+3)", "ln(5)");
  assert_parsed_expression_simplify_to("1-0.3-0.7", "0");
  assert_parsed_expression_simplify_to("(2+3-4)^(x)", "1");
  assert_parsed_expression_simplify_to("1^x", "1");
  assert_parsed_expression_simplify_to("x^1", "x");
  assert_parsed_expression_simplify_to("0^3", "0");
  assert_parsed_expression_simplify_to("0^0", "1");
  assert_parsed_expression_simplify_to("0^(-3)", "undef");
  assert_parsed_expression_simplify_to("0*x+B", "B");
  assert_parsed_expression_simplify_to("0*x*0*32*cos(3)", "0");
  assert_parsed_expression_simplify_to("1+2+0+cos(2)", "3+cos(2)");
  assert_parsed_expression_simplify_to("2+0", "2");
  assert_parsed_expression_simplify_to("3*A*B*C+4*cos(2)-2*A*B*C+A*B*C+ln(3)+4*A*B-5*A*B*C+cos(3)*ln(5)+cos(2)-45*cos(2)", "-3ABC+4AB-40cos(2)+cos(3)ln(5)+ln(3)");
  assert_parsed_expression_simplify_to("2*A+3*cos(2)+3+4*ln(5)+5*A+2*ln(5)+1+0", "7A+3cos(2)+6ln(5)+4");
  assert_parsed_expression_simplify_to("2.3*A+3*cos(2)+3+4*ln(5)+5*A+2*ln(5)+1.2+0.235", "73*A/10+3cos(2)+6ln(5)+4435/1000");
  assert_parsed_expression_simplify_to("2*A*B*C+2.3*A*B+3*A^2+5.2*A*B*C+4*A^2", "36/5*ABC+23/10*AB+7A^2");
  assert_parsed_expression_simplify_to("3*A^4*B^x*B^2*(A^2+2)*2*1.2", "36/5*A^6*B^(x+2)+72/5*A^4*B^(x+2)");
  assert_parsed_expression_simplify_to("A*(B+C)*(D+3)", "ABD+3AB+ACD+3AC");
  assert_parsed_expression_simplify_to("A/B", "A/B");
  assert_parsed_expression_simplify_to("-5P+3P", "-2P");
  assert_parsed_expression_simplify_to("(A*B)^2", "A^2*B^2");
  assert_parsed_expression_simplify_to("(A*B)^2*A+4*A^3", "A^3*B^2+4A^3");
  assert_parsed_expression_simplify_to("(A*3)^2*A+4*A^3", "13A^3");
  assert_parsed_expression_simplify_to("A^2^2*A+4*A^3", "A^5+4A^3");
  assert_parsed_expression_simplify_to("4^0.5", "2");
  assert_parsed_expression_simplify_to("8^0.5", "2R(2)");
  assert_parsed_expression_simplify_to("(12^4*3)^(0.5)", "144*R(3)");
  assert_parsed_expression_simplify_to("(2^A)^B", "2^(AB)");
  assert_parsed_expression_simplify_to("(2*A)^B", "2^B*A^B");
  assert_parsed_expression_simplify_to("(12^4*x)^(0.5)", "144R(x)");
  assert_parsed_expression_simplify_to("45^2", "2025");
  assert_parsed_expression_simplify_to("1-3+A-5+2A-4A", "-7-A");
  assert_parsed_expression_simplify_to("(1/2)*A/B", "A/(2B)");
  assert_parsed_expression_simplify_to("0.5+4*A*B-2.3+2*A*B-2*B*A^C-cos(4)+2*A^C*B+A*B*C*D", "ABCD+6AB-cos(4)-9/5");
  assert_parsed_expression_simplify_to("1+2", "3");
  assert_parsed_expression_simplify_to("123456789123456789+112233445566778899", "235690234690235688");
  assert_parsed_expression_simplify_to("1+2+3+4+5+6", "21");
  assert_parsed_expression_simplify_to("1-2+3-4+5-6", "-3");
  assert_parsed_expression_simplify_to("987654321123456789*998877665544332211", "986545842648570754445552922919330479");
  assert_parsed_expression_simplify_to("2/3", "2/3");
  assert_parsed_expression_simplify_to("9/17+5/4", "121/68");
  assert_parsed_expression_simplify_to("1/2*3/4", "3/8");
  assert_parsed_expression_simplify_to("0*2/3", "0");
  assert_parsed_expression_simplify_to("1+(1/(1+1/(1+1/(1+1))))", "8/5");
  assert_parsed_expression_simplify_to("1+2/(3+4/(5+6/(7+8)))", "155/101");
  assert_parsed_expression_simplify_to("3/4*16/12", "1");
  assert_parsed_expression_simplify_to("3/4*(8+8)/12", "1");
  assert_parsed_expression_simplify_to("916791/794976477", "305597/264992159");
  assert_parsed_expression_simplify_to("321654987123456789/112233445566778899", "3249040273974311/1133671167341201");
  assert_parsed_expression_simplify_to("0.1+0.2", "3/10");
  assert_parsed_expression_simplify_to("2^3", "8");
  assert_parsed_expression_simplify_to("(-1)*(-1)", "1");
  assert_parsed_expression_simplify_to("(-2)^2", "4");
  assert_parsed_expression_simplify_to("(-3)^3", "-27");
  assert_parsed_expression_simplify_to("(1/2)^-1", "2");
  assert_parsed_expression_simplify_to("R(32)", "4*R(2)");
  assert_parsed_expression_simplify_to("R(3^2)", "3");
  assert_parsed_expression_simplify_to("2^(2+P)", "4*2^P");
  assert_parsed_expression_simplify_to("R(15241578780673678515622620750190521)", "123456789123456789");
  assert_parsed_expression_simplify_to("R(P)^2", "P");
  assert_parsed_expression_simplify_to("R(P^2)", "P");
  assert_parsed_expression_simplify_to("R((-P)^2)", "P");
  assert_parsed_expression_simplify_to("R(x*144)", "12*R(x)");
  assert_parsed_expression_simplify_to("R(x*144*P^2)", "12*P*R(x)");
  assert_parsed_expression_simplify_to("R(x*144*P)", "12*R(xP)");
  assert_parsed_expression_simplify_to("abs(P)", "P");
  assert_parsed_expression_simplify_to("R(2)*R(3)", "R(6)");
  assert_parsed_expression_simplify_to("2*2^P", "2*2^P");
  assert_parsed_expression_simplify_to("A-A", "0");
  assert_parsed_expression_simplify_to("A-A+2cos(2)+B-B-cos(2)", "cos(2)");
  assert_parsed_expression_simplify_to("A^3*B*A^(-3)", "B");
  assert_parsed_expression_simplify_to("A^3*A^(-3)", "1");
  assert_parsed_expression_simplify_to("2^P*(1/2)^P", "1");
  assert_parsed_expression_simplify_to("A^3*A^(-3)", "1");
  assert_parsed_expression_simplify_to("1+A+2+B+3", "6+A+B");
  assert_parsed_expression_simplify_to("(x+1)*(x+2)", "x^2+3x+2");

  assert_parsed_expression_simplify_to("(x+1)*(x-1)", "-1+x^2");
  assert_parsed_expression_simplify_to("11P/(22P+11P)", "1/3");
  assert_parsed_expression_simplify_to("11/(22P+11P)", "1/(3P)");
  assert_parsed_expression_simplify_to("A^2*BA^(-2)*B^(-2)", "1/B");
  assert_parsed_expression_simplify_to("A^(-1)*B^(-1)", "1/(AB)");
  /* This does not work but should not as it is above k_primorial32 = 1*3*5*7*11*... (product of first 32 primes. */
  //assert_parsed_expression_simplify_to("1881676377434183981909562699940347954480361860897069^(1/3)", "123456789123456789");

  //assert_parsed_expression_simplify_to("1/sqrt(2)", "sqrt(2)/2");
}
