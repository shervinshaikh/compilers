#include <iostream>
#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "classhierarchy.hpp"
#include "assert.h"
#include <typeinfo>
#include <stdio.h>

/***********

    Typechecking has some code already provided, you need to fill in the blanks. You should only add code where WRITEME labels
    are inserted. You can just search for WRITEME to find all the instances.
    
    You can find descriptions of every type condition that you must check for in the project webpage. Each condition has its own
    error condition that should be thrown when it fails. Every error condition listed there and also in the "errortype" enum in
    this file must be used somewhere in your code.
    
    Be careful when throwing errors - always throw an error of the right type, using the m_attribute of the node you're visiting
    when performing the check. Sometimes you'll see errors being thrown at strange line numbers; that is okay, don't let that
    bother you as long as you follow the above principle.

*****/

class Typecheck : public Visitor {
    private:
    FILE* m_errorfile;
    SymTab* m_symboltable;
    ClassTable* m_classtable;
    
    const char * bt_to_string(Basetype bt) {
        switch (bt) {
            case bt_undef:    return "bt_undef";
            case bt_integer:  return "bt_integer";
            case bt_boolean:  return "bt_boolean";
            case bt_function: return "bt_function";
            case bt_object:   return "bt_object";
            default:
                              return "unknown";
        }
    }
    
    // the set of recognized errors
    enum errortype 
    {
        no_program,
        no_start,
        start_args_err,
        
        dup_ident_name,
        sym_name_undef,
        sym_type_mismatch,
        call_narg_mismatch,
        call_args_mismatch,
        ret_type_mismatch,
        
        incompat_assign,
        if_pred_err,
        
        expr_type_err,
        
        no_class_method,
    };
    
    // Throw errors using this method
    void t_error( errortype e, Attribute a ) 
    {
        fprintf(m_errorfile,"on line number %d, ", a.lineno );
        
        switch( e ) {
            case no_program: fprintf(m_errorfile,"error: no Program class\n"); break;
            case no_start: fprintf(m_errorfile,"error: no start function in Program class\n"); break;
            case start_args_err: fprintf(m_errorfile,"error: start function has arguments\n"); break;
            
            case dup_ident_name: fprintf(m_errorfile,"error: duplicate identifier name in same scope\n"); break;
            case sym_name_undef: fprintf(m_errorfile,"error: symbol by name undefined\n"); break;
            case sym_type_mismatch: fprintf(m_errorfile,"error: symbol by name defined, but of unexpected type\n"); break;
            case call_narg_mismatch: fprintf(m_errorfile,"error: function call has different number of args than the declaration\n"); break;
            case call_args_mismatch: fprintf(m_errorfile,"error: type mismatch in function call args\n"); break;
            case ret_type_mismatch: fprintf(m_errorfile, "error: type mismatch in return statement\n"); break;
            
            case incompat_assign: fprintf(m_errorfile,"error: types of right and left hand side do not match in assignment\n"); break;
            case if_pred_err: fprintf(m_errorfile,"error: predicate of if statement is not boolean\n"); break;
            
            case expr_type_err: fprintf(m_errorfile,"error: incompatible types used in expression\n"); break;
            
            case no_class_method: fprintf(m_errorfile,"error: function doesn't exist in object\n"); break;
            
            
            default: fprintf(m_errorfile,"error: no good reason\n"); break;
        }
        exit(1);
    }
    
    public:
    
    Typecheck(FILE* errorfile, SymTab* symboltable,ClassTable*ct) {
        m_errorfile = errorfile;
        m_symboltable = symboltable;
        m_classtable = ct;
    }
    void visitProgramImpl(ProgramImpl *p) {
    
     //WRITE ME
    }
    
    void visitClassImpl(ClassImpl *p) {
    
      //WRITE ME
    }
    
    void visitDeclarationImpl(DeclarationImpl *p) {
    
      //WRITE ME
    }
    
    void visitMethodImpl(MethodImpl *p) {
    
      //WRITE ME
    }
    
    void visitMethodBodyImpl(MethodBodyImpl *p) {
    
      //WRITE ME
    }
    
    void visitParameterImpl(ParameterImpl *p) {
    
      //WRITE ME
    }
    
    void visitAssignment(Assignment *p) {
    
      //WRITE ME
    }
    
    void visitIf(If *p) {
    
      //WRITE ME
    }
    
    void visitPrint(Print *p) {
    
      //WRITE ME
    }
    
    void visitReturnImpl(ReturnImpl *p) {
    
      //WRITE ME
    }
    
    void visitTInteger(TInteger *p) {
    
      //WRITE ME
    }
    
    void visitTBoolean(TBoolean *p) {
    
      //WRITE ME
    }
    
    void visitTNothing(TNothing *p) {
    
      //WRITE ME
    }
    
    void visitTObject(TObject *p) {
    
      //WRITE ME
    }
    
    void visitClassIDImpl(ClassIDImpl *p) {
    
      //WRITE ME
    }
    
    void visitVariableIDImpl(VariableIDImpl *p) {
    
      //WRITE ME
    }
    
    void visitMethodIDImpl(MethodIDImpl *p) {
    
      //WRITE ME
    }
    
    void visitPlus(Plus *p) {
    
      //WRITE ME
    }
    
    void visitMinus(Minus *p) {
    
      //WRITE ME
    }
    
    void visitTimes(Times *p) {
    
      //WRITE ME
    }
    
    void visitDivide(Divide *p) {
    
      //WRITE ME
    }
    
    void visitAnd(And *p) {
    
      //WRITE ME
    }
    
    void visitLessThan(LessThan *p) {
    
      //WRITE ME
    }
    
    void visitLessThanEqualTo(LessThanEqualTo *p) {
    
      //WRITE ME
    }
    
    void visitNot(Not *p) {
    
      //WRITE ME
    }
    
    void visitUnaryMinus(UnaryMinus *p) {
    
      //WRITE ME
    }
    
    void visitMethodCall(MethodCall *p) {
    
      //WRITE ME
    }
    
    void visitSelfCall(SelfCall *p) {
    
      //WRITE ME
    }
    
    void visitVariable(Variable *p) {
    
      //WRITE ME
    }
    
    void visitIntegerLiteral(IntegerLiteral *p) {
    
      //WRITE ME
    }
    
    void visitBooleanLiteral(BooleanLiteral *p) {
    
      //WRITE ME
    }
    
    void visitNothing(Nothing *p) {
    
      //WRITE ME
    }
    
    void visitSymName(SymName *p) {
    
      //WRITE ME
    }
    
    void visitPrimitive(Primitive *p) {
    
      //WRITE ME
    }
    
    void visitClassName(ClassName *p) {
    
      //WRITE ME
    }
    
    
    void visitNullPointer() {}
};
