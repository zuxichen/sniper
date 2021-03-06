/**
 * \file Symbol.h
 *
 * ----------------------------------------------------------------------
 *                SNIPER : Automatic Fault Localization 
 *
 * Copyright (C) 2016 Si-Mohamed LAMRAOUI
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program (see LICENSE.TXT).  
 * If not, see <http://www.gnu.org/licenses/>.
 * ----------------------------------------------------------------------
 *
 * \author Si-Mohamed Lamraoui
 * \date   22 February 2016
 */

#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <memory>

#include "llvm/IR/Constants.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/InstrTypes.h"

#include "Logic/Expression.h"

using namespace llvm;

class Symbol;
class InputSymbol;
class BinaryOpSymbol;
class SSBinaryOpSymbol;
class VSBinaryOpSymbol;
class SVBinaryOpSymbol;
class SelectOpSymbol;
class SSSelectOpSymbol;
class VSSelectOpSymbol;
class SVSelectOpSymbol;
class CastOpSymbol;
class GepOpSymbol;

typedef std::shared_ptr<Symbol> SymbolPtr;
typedef std::shared_ptr<InputSymbol> InputSymbolPtr;
typedef std::shared_ptr<BinaryOpSymbol> BinaryOpSymbolPtr;
typedef std::shared_ptr<SSBinaryOpSymbol> SSBinaryOpSymbolPtr;
typedef std::shared_ptr<VSBinaryOpSymbol> VSBinaryOpSymbolPtr;
typedef std::shared_ptr<SVBinaryOpSymbol> SVBinaryOpSymbolPtr;
typedef std::shared_ptr<SelectOpSymbol> SelectOpSymbolPtr;
typedef std::shared_ptr<SSSelectOpSymbol> SSSelectOpSymbolPtr;
typedef std::shared_ptr<VSSelectOpSymbol> VSSelectOpSymbolPtr;
typedef std::shared_ptr<SVSelectOpSymbol> SVSelectOpSymbolPtr;
typedef std::shared_ptr<CastOpSymbol> CastOpSymbolPtr;
typedef std::shared_ptr<GepOpSymbol> GepOpSymbolPtr;


/**
 * \class Symbol
 *
 * \brief This class represents symbolic variables. 
 *
 * In concolic execution (see ConcolicModule), 
 * variables are treated as symbolic variables during 
 * symbolic execution. 
 */
class Symbol {
    
protected:
    /**
     * Unique ID number to be assigned to each symbol.
     */
    static unsigned ID;
    unsigned currentID;
    /**
     * LLVM value to be represented.
     */
    Value *v;

public:
    /**
     * Default constructor.
     *
     * \param _v An LLVM value.
     */
    Symbol(Value *_v) : v(_v), currentID(ID++) {
        assert(v && "No value for symbol!");
    }
    /**
     * Destructor.
     */
    virtual ~Symbol() { }

    /**
     * Convert this symbol into a logic formula.
     *
     * \return An expression.
     */
    virtual ExprPtr convertToExpr();
    /**
     * Return the represented value.
     */
    Value* getValue() { 
        return v; 
    }
    /**
     * Return true if this symbol is an input symbol, 
     * otherwise false.
     *
     * An input symbol is a symbol that represents an 
     * argument of the target function.
     */
    virtual bool isInput() { 
        return false; 
    }
    /**
     * Dump to the standard output this symbol.  
     */
    virtual void dump() {
        std::cout << "S(";
        Symbol::dump(v);
        std::cout << ")";
        std::cout.flush();
    }

    /**
     * Dump to the standard ouput the 
     * represented variable.
     */
    static void dump(Value *v);
    /**
     * Return a textual representation of an 
     * LLVM comparison predicate. 
     */
    static std::string getPredicateStr(CmpInst::Predicate p);
};

/**
 * \class InputSymbol
 *
 * \brief An input symbol is a symbol that represents an 
 * argument of the target function.
 *
 * See ConcolicModule for more details. 
 */
class InputSymbol : public Symbol {
public:
    InputSymbol(Value *v) : Symbol(v) { }
    /**
     * Always return true since this class is an input symbol.
     */
    bool isInput() { 
        return true; 
    }
    virtual void dump() {
        if(v==NULL) {
            std::cout << "null";
        } else {
            std::cout << "I(" << v->getName().str() << ")";
        }
        std::cout.flush(); 
    }
};

/**
 * \class BinaryOpSymbol
 *
 * \brief A binary op symbol is a symbol that represents 
 * a binary instruction (add, sub, ...).
 */
class BinaryOpSymbol : public Symbol {
protected:
    BinaryOpSymbol(Value *_v) : Symbol(_v) { }
};

/**
 * \class SSBinaryOpSymbol
 *
 * \brief This class is a binary op symbol with 
 * two of its arguments being symbolic variables. 
 *
 * S(v)= S(v1) op S(v2)
 */
class SSBinaryOpSymbol : public BinaryOpSymbol {
protected:
    SymbolPtr s1; // First argument
    SymbolPtr s2; // Second argument
public:
    SSBinaryOpSymbol(Value *v, SymbolPtr s1, SymbolPtr s2) 
    : BinaryOpSymbol(v), s1(s1), s2(s2) { }
    virtual ExprPtr convertToExpr() {
        if (Instruction *I = dyn_cast<Instruction>(v)) {
            ExprPtr e1 = s1->convertToExpr();
            ExprPtr e2 = s2->convertToExpr();
            return Expression::mkOp(e1, e2, I);
        } else {
            std::cout << "error: symbol wihout instruction.\n";
            exit(1);
        }
    }
    virtual void dump() {
        std::cout << "SSB(";
        s1->dump();
        std::cout << " op ";
        s2->dump();
        std::cout.flush(); 
    }
};

/**
 * \class SVBinaryOpSymbol
 *
 * \brief This class is a binary op symbol with 
 * the first argument being a symbolic variable 
 * and the second argument being a concrete value.
 *
 * S(v)= S(v1) op v2
 */
class SVBinaryOpSymbol : public BinaryOpSymbol {
protected:
    SymbolPtr s1; // First argument
    Value  *v2;   // Second argument (concrete value)
public:
    SVBinaryOpSymbol(Value *v, SymbolPtr s1, Value *v2) 
    : BinaryOpSymbol(v), s1(s1), v2(v2) { }
    virtual ExprPtr convertToExpr() {
        Instruction *I = dyn_cast<Instruction>(v);
        assert(I && "Symbol has no instruction!");
        ExprPtr e1 = s1->convertToExpr();
        ExprPtr e2 = Expression::getExprFromValue(v2);
        return Expression::mkOp(e1, e2, I);
    }
    virtual void dump() {
        std::cout << "SVB(";
        s1->dump();
        std::cout << " op ";
        Symbol::dump(v2);
        std::cout << ")";
        std::cout.flush(); 
    }
};

/**
 * \class VSBinaryOpSymbol
 *
 * \brief This class is a binary op symbol with 
 * the first argument being a concrete value 
 * and the second argument being a symbolic variable. 
 *
 * S(v)= v1 op S(v2)
 */
class VSBinaryOpSymbol : public BinaryOpSymbol {
protected:
    Value *v1;    // First argument (concrete value)
    SymbolPtr s2; // Second argument
public:
    VSBinaryOpSymbol(Value *v, Value *v1, SymbolPtr s2) 
    : BinaryOpSymbol(v), v1(v1), s2(s2) { }
    virtual ExprPtr convertToExpr() {
        Instruction *I = dyn_cast<Instruction>(v);
        assert(I && "Symbol has no instruction!");
        ExprPtr e1 = Expression::getExprFromValue(v1);
        ExprPtr e2 = s2->convertToExpr();
        return Expression::mkOp(e1, e2, I);
    }
    virtual void dump() {
        std::cout << "SVB(";
        Symbol::dump(v1);
        std::cout << " op ";
        s2->dump(); 
        std::cout << ")";
        std::cout.flush(); 
    }
};

/**
 * \class SelectOpSymbol
 *
 * \brief A select symbol is a symbol that represents 
 * a select instruction (a?b:c).
 */
class SelectOpSymbol : public Symbol {
protected:
    SymbolPtr s1;
public:
    SelectOpSymbol(Value *v, SymbolPtr s1) : Symbol(v), s1(s1) { }
};

/**
 * \class SSSelectOpSymbol
 *
 * \brief This class is a select symbol with all   
 * of its arguments being symbolic variables. 
 *
 * S(v)= S(v1) ? S(v2) : S(v3)
 */
class SSSelectOpSymbol : public SelectOpSymbol {
protected:
    SymbolPtr s2; // Second argument
    SymbolPtr s3; // Third argument
public:
    SSSelectOpSymbol(Value *v, SymbolPtr s1, SymbolPtr s2, SymbolPtr s3) 
    : SelectOpSymbol(v, s1), s2(s2), s3(s3) { }
    virtual ExprPtr convertToExpr() {
        assert(isa<SelectInst>(v) && "Value is not a select instruction!");
        ExprPtr e1 = s1->convertToExpr();
        ExprPtr e2 = s2->convertToExpr();
        ExprPtr e3 = s3->convertToExpr();
        return Expression::mkIte(e1, e2, e3);
    }
    virtual void dump() {
        std::cout << "SSS(";
        s1->dump();
        std::cout << " ? ";
        s2->dump();
        std::cout << " : ";
        s3->dump();
        std::cout.flush(); 
    }
};

/**
 * \class VSSelectOpSymbol
 *
 * \brief This class is a select symbol with the 
 * second argument being a concrete value and    
 * the other arguments being symbolic variables. 
 *
 * S(v)= S(v1) ? v2 : S(v3)
 */
class VSSelectOpSymbol : public SelectOpSymbol {
protected:
    Value *v2;    // Second arument (concrete value)
    SymbolPtr s3; // Third argument
public:
    VSSelectOpSymbol(Value *v, SymbolPtr s1, Value *v2, SymbolPtr s3) 
    : SelectOpSymbol(v, s1), v2(v2), s3(s3) { }
    virtual ExprPtr convertToExpr() {
        assert(isa<SelectInst>(v) && "Value is not a select instruction!");
        ExprPtr e1 = s1->convertToExpr();
        ExprPtr e2 = Expression::getExprFromValue(v2);
        ExprPtr e3 = s3->convertToExpr();
        return Expression::mkIte(e1, e2, e3);
    }
    virtual void dump() {
        std::cout << "VSS(";
        s1->dump();
        std::cout << " ? ";
        Symbol::dump(v2);
        std::cout << " : ";
        s3->dump();
        std::cout.flush(); 
    }
};

/**
 * \class SVSelectOpSymbol
 *
 * \brief This class is a select symbol with the 
 * last argument being a concrete value and    
 * the other arguments being symbolic variables. 
 *
 * S(v)= S(v1) ? S(v2) : v3
 */
class SVSelectOpSymbol : public SelectOpSymbol {
protected:
    SymbolPtr s2; // Second argument
    Value *v3;    // Third argument (concrete value) 
public:
    SVSelectOpSymbol(Value *v, SymbolPtr s1, SymbolPtr s2, Value *v3) 
    : SelectOpSymbol(v, s1), s2(s2), v3(v3) { }
    virtual ExprPtr convertToExpr() {
        assert(isa<SelectInst>(v) && "Value is not a select instruction!");
        ExprPtr e1 = s1->convertToExpr();
        ExprPtr e2 = s2->convertToExpr();
        ExprPtr e3 = Expression::getExprFromValue(v3);
        return Expression::mkIte(e1, e2, e3);
    }
    virtual void dump() {
        std::cout << "SVS(";
        s1->dump();
        std::cout << " ? ";
        s2->dump();
        std::cout << " : ";
        Symbol::dump(v3);
        std::cout.flush(); 
    }
};

/**
 * \class VVSelectOpSymbol
 *
 * \brief This class is a select symbol with the  
 * first argument being a symbolic variable and    
 * the other arguments being concrete values. 
 *
 * S(v)= S(v1) ? v2 : v3
 */
class VVSelectOpSymbol : public SelectOpSymbol {
protected:
    Value *v2; // Second argument (concrete value)
    Value *v3; // Third argument (concrete value)
public:
    VVSelectOpSymbol(Value *v, SymbolPtr s1, Value *v2, Value *v3) 
    : SelectOpSymbol(v, s1), v2(v2), v3(v3) { }
    virtual ExprPtr convertToExpr() {
       assert(isa<SelectInst>(v) && "Value is not a select instruction!");
        ExprPtr e1 = s1->convertToExpr();
        ExprPtr e2 = Expression::getExprFromValue(v2);
        ExprPtr e3 = Expression::getExprFromValue(v3);
        return Expression::mkIte(e1, e2, e3);
    }
    virtual void dump() {
        std::cout << "SSS(";
        s1->dump();
        std::cout << " ? ";
        Symbol::dump(v2);
        std::cout << " : ";
        Symbol::dump(v3);
        std::cout.flush(); 
    }
};

/**
 * \class CastOpSymbol
 *
 * \brief A cast op symbol is a symbol that represents 
 * a cast instruction.
 *
 * S(v)= cast S(v1)
 */
class CastOpSymbol : public Symbol {
protected:
    SymbolPtr s1; // Argument to be casted
public:
    CastOpSymbol(Value *v, SymbolPtr s1) 
    : Symbol(v), s1(s1) { }
    virtual ExprPtr convertToExpr() {
        Instruction *I = dyn_cast<Instruction>(v);
        assert(I && "Symbol has no instruction!");
        Value *v1  = I->getOperand(0);
        Type *vTy = v->getType();
        Type *v1Ty = v1->getType();
        // i1 -> i32
        if (v1Ty->isIntegerTy(1) && vTy->isIntegerTy(32)) {
            // v = IF v1 THEN 1 ELSE 0
            ExprPtr one = Expression::mkSInt32Num(1);
            ExprPtr zero = Expression::mkSInt32Num(0);
            ExprPtr e1 = s1->convertToExpr();
            return Expression::mkIte(e1, one, zero);
        }
        // i32 -> i1
        else if (v1Ty->isIntegerTy(32) && vTy->isIntegerTy(1)) {
            // v = (!= v1 0) : IF v1!=0 THEN true ELSE false
            ExprPtr zero = Expression::mkSInt32Num(0);
            ExprPtr e1 = s1->convertToExpr();
            return Expression::mkDiseq(e1, zero);
        } else {
            assert("Unsupported cast operation!");
        }
    }
    virtual void dump() {
        std::cout << "SVB(cast ";
        s1->dump();
        std::cout << ")";
        std::cout.flush(); 
    }
};

/**
 * \class GepOpSymbol
 *
 * \brief A gep op symbol is a symbol that represents 
 * a get pointer element instruction.
 *
 * P(ptr) = gep *a, S(v1)
 *
 * Limitation: only support integer values
 *             (*a cannot be symbolic).
 */
class GepOpSymbol : public Symbol {
protected:
    Value *a;     // Address argument (concrete value)
    SymbolPtr s1; // Offset argument
public:
    GepOpSymbol(Value *ptr, Value *a, SymbolPtr s1) 
    : Symbol(ptr), a(a), s1(s1) { }
    virtual ExprPtr convertToExpr() {
        // (a s1)
        ExprPtr e1 = Expression::getExprFromValue(a);
        ExprPtr e2 = s1->convertToExpr();
        ExprPtr e3 = Expression::mkApp(e1, e2);
        /*ExprPtr e4 = NULL;
        // (and (>= s1 0) (< s1 size))
        if(AllocaInst *alloc = dyn_cast<AllocaInst>(a)) {
            Type *ty = a->getAllocatedType();
            if (ty->isArrayTy()) {
                // (>= s1 0)
                ExprPtr zero = Expression::mkNum(0);
                ExprPtr geq = Expression::mkGe(e2, zero);
                // (< s1 size)
                unsigned size = ty->getArrayNumElements();
                ExprPtr esize = Expression::mkNum(size);                
                ExprPtr lt = Expression::mkLt(e2, esize);
                e4 = Expression::mkAnd(geq, lt); 
            }
        }*/
        return e3;
    }
    virtual void dump() {
        std::cout << "SGEP( ";
        Symbol::dump(a);
        std::cout << " ";
        s1->dump();
        std::cout << ")";
        std::cout.flush(); 
    }
};

#endif // _SYMBOL_H
