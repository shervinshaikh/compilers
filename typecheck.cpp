#include <iostream>
#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "classhierarchy.hpp"
#include "assert.h"
#include <typeinfo>
#include <stdio.h>

#define forall(iterator,listptr) \
  for(iterator = listptr->begin(); iterator != listptr->end(); iterator++) \
  
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
    ClassName* current_class_name;
    bool just_return;
    
    const char * bt_to_string(Basetype bt) {
        switch (bt) {
            case bt_undef:    return "bt_undef";
            case bt_integer:  return "bt_integer";
            case bt_boolean:  return "bt_boolean";
            case bt_function: return "bt_function";
            case bt_object:   return "bt_object";
            case bt_nothing:  return "bt_nothing";
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
      just_return = true;
      list<Class_ptr>::iterator class_i;
      forall(class_i, p->m_class_list){
        m_symboltable->open_scope();
        ClassImpl* c = ((ClassImpl*)(*class_i));
        char* className = strdup(((ClassIDImpl*)c->m_classid_1)->m_classname->spelling());

        char* superClass = NULL;
        if((c->m_classid_2) != NULL){
          superClass = strdup(((ClassIDImpl*)c->m_classid_2)->m_classname->spelling());
          if(m_classtable->exist(superClass)){
            // if superclass than add it below that scope? how else we'll we be able to check stuff
            // SymScope *scope = new SymScope();
            // scope = m_symboltable->get_current_scope();
            m_classtable->insert(new ClassName(className), new ClassName(superClass), c, m_symboltable->get_current_scope());

            current_class_name = ((ClassIDImpl*)c->m_classid_1)->m_classname;

            visitClassImpl(c);

            // m_symboltable->get_current_scope()->add_child(scope);
          } else {
            this->t_error(sym_name_undef, p->m_attribute);
          }
        } else {
          // 4. No two classes may have the same name
          if(m_classtable->exist(className))
            this->t_error(dup_ident_name, p->m_attribute);

          m_classtable->insert(new ClassName(className), NULL, c, m_symboltable->get_current_scope());

          current_class_name = ((ClassIDImpl*)c->m_classid_1)->m_classname;

          visitClassImpl(c);
        }
        m_symboltable->close_scope();
      }


      // p->visit_children(this);
      FILE *symFile;
      symFile = fopen("symboltable.txt" , "w");
      m_symboltable->dump(symFile);
      
      // 1. Every input program is required to have a class called "Program"
      // This class must appear as the last class in the program.
      char *programName = strdup(p->m_class_list->back()->m_attribute.m_type.classType.classID);
      if(strcmp(programName, "Program") != 0)
        this->t_error(no_program, p->m_attribute);

      ClassNode *program = m_classtable->lookup(programName);
      bool foundStart = false;
      list<Method_ptr>::iterator meth_i;
      forall(meth_i, program->p->m_method_list){
        // 2. The required Program class must have a method called "start".
        // TODO: This start method must return the type Nothing.
        if(strcmp((*meth_i)->m_attribute.m_type.classType.classID, "start") == 0 && (*meth_i)->m_attribute.m_type.methodType.returnType.baseType == bt_nothing){
          foundStart = true;

          // 3. The required "start" method takes exactly zero parameters.
          if((*meth_i)->m_attribute.m_type.methodType.argsType.size() > 0)
            this->t_error(start_args_err, p->m_attribute);
        }

      }
      if(!foundStart)
        this->t_error(no_start, p->m_attribute);
      
      
    }
    
    void visitClassImpl(ClassImpl *p) {
    
      //WRITE ME
      // m_symboltable->open_scope();
      p->visit_children(this);

      char* name = strdup(p->m_classid_1->m_attribute.m_type.classType.classID);
      p->m_attribute.m_type.classType.classID = strdup(name);

      // m_symboltable->close_scope();
    }
    
    void visitDeclarationImpl(DeclarationImpl *p) {
    
      //WRITE ME
      p->visit_children(this);

      Basetype type = p->m_type->m_attribute.m_type.baseType;
      char* className = NULL;
      if(type == bt_object){
        className = strdup(p->m_type->m_attribute.m_type.classType.classID);
      }

      list<VariableID_ptr>::iterator var_i;
      forall(var_i, p->m_variableid_list){
        Symbol *s = new Symbol();
        s->baseType = type;
        if(className != NULL)
          s->classType.classID = className;
        char *name = strdup((*var_i)->m_attribute.m_type.classType.classID);

        // cerr << "declared: " << name << ", type: " << bt_to_string(type) << endl;
        // 6. Two properties of the same class cannot have the same name (same class var names)
        // 7. Two local variables of the same method cannot have the same name (same local var names)        
        // 8. Local variable of a method and a property of the class containing that method cannot have the same name (same class var and method var names)
        if(m_symboltable->exist(name))
          this->t_error(dup_ident_name, p->m_attribute);
        else
          m_symboltable->insert(name, s);
      }
    }
    
    void visitMethodImpl(MethodImpl *p) {
    
      //WRITE ME
      m_symboltable->open_scope();
      p->visit_children(this);
      // visitMethodIDImpl((MethodIDImpl*)p->m_methodid);

      Symbol *s = new Symbol();
      char* methodName = strdup(p->m_methodid->m_attribute.m_type.classType.classID);
      p->m_attribute.m_type.classType.classID = strdup(methodName);

      list<Parameter_ptr>::iterator par_i;
      forall(par_i, p->m_parameter_list){
        // visitParameterImpl((ParameterImpl*)(*par_i));
        CompoundType parameter;
        parameter.baseType = (*par_i)->m_attribute.m_type.classType.baseType;
        parameter.classID = (*par_i)->m_attribute.m_type.classType.classID;

        Symbol *s2 = m_symboltable->lookup(parameter.classID);
        s->classType.classID = s2->classType.classID;
        parameter.classID = s->classType.classID;

        p->m_attribute.m_type.methodType.argsType.push_back(parameter);
        s->methodType.argsType.push_back(parameter);
      }

      // Visit return type
      // p->m_type->accept(this);
      // just_return = true;
      // visitMethodBodyImpl((MethodBodyImpl*)p->m_methodbody);

      Basetype type = bt_function;
      s->baseType = type;
      p->m_attribute.m_type.baseType = type;

      Basetype returnType = p->m_methodbody->m_attribute.m_type.baseType;
      s->methodType.returnType.baseType = returnType;
      p->m_attribute.m_type.methodType.returnType.baseType = returnType;

      // 16. Declared Return Type Must Match Type of Return Statement (error: ret_type_mismatch)
      if(p->m_type->m_attribute.m_type.baseType != returnType){
        // cerr << p->m_type->m_attribute.m_type.baseType << " " << returnType << endl;
        this->t_error(ret_type_mismatch, p->m_attribute);
      }

      // 5. Two methods in the same class cannot have the same name
      if(m_symboltable->exist(methodName))
        this->t_error(dup_ident_name, p->m_attribute);
      else
        m_symboltable->insert_in_parent_scope(methodName, s);

      // just_return = false;
      // visitMethodBodyImpl((MethodBodyImpl*)p->m_methodbody);

      m_symboltable->close_scope();
    }
    
    void visitMethodBodyImpl(MethodBodyImpl *p) {
    
      //WRITE ME
      p->visit_children(this);

      // if(just_return){
      //   p->m_return->accept(this);
        p->m_attribute.m_type.baseType = p->m_return->m_attribute.m_type.baseType;  
      // } else {
      //   list<Declaration_ptr>::iterator dec_i;
      //   forall(dec_i, p->m_declaration_list){
      //     visitDeclarationImpl((DeclarationImpl*)*dec_i);
      //   }

      //   list<Statement_ptr>::iterator stat_i;
      //   forall(stat_i, p->m_statement_list){
      //     Assignment* a = ((Assignment*)*stat_i);
      //     If *i = ((If*)*stat_i);
      //     Print *p = ((Print*)*stat_i);

      //     // Assignment?
      //     if(a->m_variableid != NULL && a->m_expression != NULL){
      //       visitAssignment(a);
      //     }
      //     // If?
      //     else if(i->m_expression != NULL && i->m_statement != NULL){
      //       visitIf(i);
      //     } 
      //     // Print?
      //     else if(p->m_expression != NULL){
      //       visitPrint(p);
      //     }
      //   }
      // }

      // p->m_attribute.m_type.methodType.returnType.classID = p->m_return->m_attribute.m_type;
    }
    
    void visitParameterImpl(ParameterImpl *p) {
    
      //WRITE ME
      p->visit_children(this);

      char* paramName = strdup(p->m_variableid->m_attribute.m_type.classType.classID);
      p->m_attribute.m_type.classType.classID = strdup(paramName);

      Basetype type = p->m_type->m_attribute.m_type.baseType;
      p->m_attribute.m_type.classType.baseType = type;

      char* className = NULL;
      if(type == bt_object){
        className = strdup(p->m_type->m_attribute.m_type.classType.classID);
      }

      Symbol *s = new Symbol();
      s->baseType = type;
      if(className != NULL){
        s->classType.classID = className;
        // cerr << s->classType.classID << endl;
      }

      // 7. Two local variables of the same method cannot have the same name (same local var names)
      if(m_symboltable->exist(paramName))
        this->t_error(dup_ident_name, p->m_attribute);
      else
        m_symboltable->insert(paramName, s);
    }
    
    void visitAssignment(Assignment *p) {
    
      //WRITE ME
      p->visit_children(this);

      char *varName = strdup(p->m_variableid->m_attribute.m_type.classType.classID);
      // p->m_attribute.m_type.classType.classID = strdup(varName);
      Symbol *s = m_symboltable->lookup(varName);

      // 9. No Usage of Undefined Variables (error: sym_name_undef)
      if(!m_symboltable->exist(varName)){
        // TODO need access to current class name
        char* className = strdup(current_class_name->spelling());
        // cerr << className << endl;

        ClassNode* current_class = m_classtable->lookup(className);

        bool foundInParentClass = false;
        // ClassNode
        while( strcmp(current_class->name->spelling(), "TopClass") != 0 && current_class->scope != NULL){
          s = current_class->scope->lookup(varName);
          if(s != NULL){
            foundInParentClass = true;
            break;
          }
          current_class = m_classtable->getParentOf(current_class->name);
        }
        if(!foundInParentClass){
          // cerr << "ASDASDASD" << endl;
          t_error(sym_name_undef, p->m_attribute);
        }
      }

      p->m_attribute.m_type.baseType = s->baseType;

      Basetype left = s->baseType;
      Basetype right = p->m_expression->m_attribute.m_type.baseType;

      // cerr << "Assignment| " << varName << " " << bt_to_string(left) << " = " << bt_to_string(right) << endl;

      // 18. If both sides are of Object type, then the right-hand-side type may be a subclass of the left-hand-side type.
      if(left == bt_object && right == bt_object){
        ClassNode *c = m_classtable->lookup((char*)s->classType.classID);
        if(c == NULL){
          // cerr << "here" << endl;
          this->t_error(sym_type_mismatch, p->m_attribute);
        }
        // cerr << "Match? " << c->name->spelling() << endl;

        char* className = strdup(p->m_expression->m_attribute.m_type.classType.classID);

        // cerr << (char*)s->classType.classID << endl;
        ClassNode *c2 = m_classtable->lookup(className);
        if(c2 == NULL){
          // cerr << "sdfshere" << endl;
          this->t_error(sym_type_mismatch, p->m_attribute);
        }
        // cerr << "Subclass? " << c2->name->spelling() << endl;
        if(strcmp(c->name->spelling(), c2->name->spelling()) != 0){
          // if(c2->p->m_classid_2 != NULL)
          //   cerr << " superclass: " << c2->p->m_classid_2->m_attribute.m_type.classType.classID << endl;

          bool foundInParentClass = false;
          while(c2->p->m_classid_2 != NULL){
            if(strcmp(c->name->spelling(), (char*)c2->p->m_classid_2->m_attribute.m_type.classType.classID) == 0){
              foundInParentClass = true;
            }
            c2 = m_classtable->lookup((char*)c2->p->m_classid_2->m_attribute.m_type.classType.classID);
          }
          if(foundInParentClass == false){
            this->t_error(incompat_assign ,p->m_attribute);
          }
        }
      }

      // 17. In every assignment statement, the left and right hand sides must have the same type.
      // This means that the type of the left-hand-side variable and the right-hand-side expression must be equivalent.
      if(left != right){
        this->t_error(incompat_assign, p->m_attribute);
      }

      p->m_attribute.m_type.baseType = bt_integer;
    }
    
    void visitIf(If *p) {
      p->visit_children(this);

      // 19. If Predicate Expression Must be Bool Type (error: if_pred_err)
      if(p->m_expression->m_attribute.m_type.baseType != bt_boolean)
        this->t_error(if_pred_err, p->m_attribute);
    }
    
    void visitPrint(Print *p) {
      p->visit_children(this);
    }
    
    void visitReturnImpl(ReturnImpl *p) {
      p->visit_children(this);

      p->m_attribute.m_type.baseType = p->m_expression->m_attribute.m_type.baseType;
    }
    
    void visitTInteger(TInteger *p) {
      p->m_attribute.m_type.baseType = bt_integer;
    }
    
    void visitTBoolean(TBoolean *p) {
      p->m_attribute.m_type.baseType = bt_boolean;
    }
    
    void visitTNothing(TNothing *p) {
      p->m_attribute.m_type.baseType = bt_nothing;
    }
    
    void visitTObject(TObject *p) {
      p->m_attribute.m_type.baseType = bt_object;
      ClassIDImpl* impl = (ClassIDImpl*) p->m_classid;
      p->m_attribute.m_type.classType.classID = strdup(impl->m_classname->spelling());
    }
    
    void visitClassIDImpl(ClassIDImpl *p) {
      p->m_attribute.m_type.classType.classID = strdup(p->m_classname->spelling());
    }
    
    void visitVariableIDImpl(VariableIDImpl *p) {
      p->m_attribute.m_type.classType.classID = strdup(p->m_symname->spelling());
    }
    
    void visitMethodIDImpl(MethodIDImpl *p) {
      p->m_attribute.m_type.classType.classID = strdup(p->m_symname->spelling());
      p->m_attribute.m_type.baseType = bt_function;
    }
    
    // 20. Addition, Subtraction, Multiplication, Division, and Unary Minus expressions must have Int operands and they all produce Int.
    void visitPlus(Plus *p) {
      p->visit_children(this);

      Basetype left = p->m_expression_1->m_attribute.m_type.baseType;
      Basetype right = p->m_expression_2->m_attribute.m_type.baseType;

      if(left != bt_integer || right != bt_integer)
        this->t_error(expr_type_err, p->m_attribute);

      p->m_attribute.m_type.baseType = bt_integer;
    }
    
    void visitMinus(Minus *p) {
      p->visit_children(this);

      Basetype left = p->m_expression_1->m_attribute.m_type.baseType;
      Basetype right = p->m_expression_2->m_attribute.m_type.baseType;

      if(left != bt_integer || right != bt_integer)
        this->t_error(expr_type_err, p->m_attribute);

      p->m_attribute.m_type.baseType = bt_integer;
    }
    
    void visitTimes(Times *p) {
      p->visit_children(this);

      Basetype left = p->m_expression_1->m_attribute.m_type.baseType;
      Basetype right = p->m_expression_2->m_attribute.m_type.baseType;

      if(left != bt_integer || right != bt_integer)
        this->t_error(expr_type_err, p->m_attribute);

      p->m_attribute.m_type.baseType = bt_integer;
    }
    
    void visitDivide(Divide *p) {
      p->visit_children(this);

      Basetype left = p->m_expression_1->m_attribute.m_type.baseType;
      Basetype right = p->m_expression_2->m_attribute.m_type.baseType;

      if(left != bt_integer || right != bt_integer)
        this->t_error(expr_type_err, p->m_attribute);

      p->m_attribute.m_type.baseType = bt_integer;
    }
    
    //  22. And and Not expressions must have Bool operands and they both produce Bool.
    void visitAnd(And *p) {
      p->visit_children(this);

      Basetype left = p->m_expression_1->m_attribute.m_type.baseType;
      Basetype right = p->m_expression_2->m_attribute.m_type.baseType;

      if(left != bt_boolean || right != bt_boolean)
        this->t_error(expr_type_err, p->m_attribute);

      p->m_attribute.m_type.baseType = bt_boolean;
    }
    
    // 21. Less Than and Less or Equal to expressions must have Int operands and they both produce Bool.
    void visitLessThan(LessThan *p) {
      p->visit_children(this);

      Basetype left = p->m_expression_1->m_attribute.m_type.baseType;
      Basetype right = p->m_expression_2->m_attribute.m_type.baseType;

      if(left != bt_integer || right != bt_integer)
        this->t_error(expr_type_err, p->m_attribute);

      p->m_attribute.m_type.baseType = bt_boolean;
    }
    
    void visitLessThanEqualTo(LessThanEqualTo *p) {
      p->visit_children(this);

      Basetype left = p->m_expression_1->m_attribute.m_type.baseType;
      Basetype right = p->m_expression_2->m_attribute.m_type.baseType;

      if(left != bt_integer || right != bt_integer)
        this->t_error(expr_type_err, p->m_attribute);

      p->m_attribute.m_type.baseType = bt_boolean;
    }
    
    void visitNot(Not *p) {
      p->visit_children(this);

      if(p->m_expression->m_attribute.m_type.baseType != bt_boolean)
        this->t_error(expr_type_err, p->m_attribute);

      p->m_attribute.m_type.baseType = bt_boolean;
    }
    
    void visitUnaryMinus(UnaryMinus *p) {
      p->visit_children(this);

      if(p->m_expression->m_attribute.m_type.baseType != bt_integer)
        this->t_error(expr_type_err, p->m_attribute);

      p->m_attribute.m_type.baseType = bt_integer;
    }
    
    void visitMethodCall(MethodCall *p) {
    
      //WRITE ME
      p->visit_children(this);
      // cerr << "method call" << endl;

      // 10. Identifiers which are used as method names must have the method type
      if(p->m_methodid->m_attribute.m_type.baseType != bt_function){
        this->t_error(sym_type_mismatch, p->m_attribute);
      }

      list<Expression_ptr>::iterator exp_i;
      forall(exp_i, p->m_expression_list){
        if((*exp_i)->m_attribute.m_type.baseType == bt_function){
          this->t_error(sym_type_mismatch, p->m_attribute);    
        }
      }

      char* className = (char*)m_symboltable->lookup(p->m_variableid->m_attribute.m_type.classType.classID)->classType.classID;
      char* methodName = strdup(p->m_methodid->m_attribute.m_type.classType.classID);

      ClassNode* current_class = m_classtable->lookup(className);
      Symbol *s = current_class->scope->lookup(methodName);

      bool foundInParentClass = false;
      while(strcmp(current_class->name->spelling(), "TopClass") != 0 && current_class->scope != NULL){
        // cerr << current_class->name->spelling() << endl;
        s = current_class->scope->lookup(methodName);
        if(s != NULL){
          foundInParentClass = true;
          break;
        }
        current_class = m_classtable->getParentOf(current_class->name);
      }

      if(foundInParentClass){
        // cerr << p->m_expression_list->size() << " " << s->methodType.argsType.size() << endl;
        // 14. Number of Arguments Must Match Number of Parameters (error: call_narg_mismatch)
        if(s->methodType.argsType.size() != p->m_expression_list->size()){
          this->t_error(call_narg_mismatch, p->m_attribute);
        } else {
          // 15. Type of Arguments Must Match Type of Parameters (error: call_args_mismatch)
          list<Expression_ptr>::iterator exp_i2;
          vector<CompoundType>::iterator param_i;
          param_i = s->methodType.argsType.begin();
          forall(exp_i2, p->m_expression_list){
            Basetype argT = (*exp_i2)->m_attribute.m_type.baseType;
            Basetype paramT = param_i->baseType;

            if(argT == bt_object && paramT == bt_object){
              char* arg = strdup((*exp_i2)->m_attribute.m_type.classType.classID);
              char* param = strdup(param_i->classID);
              // cerr << arg << param << endl;
              if(strcmp(arg, param) != 0){
                // Loop through subclass to see if name matches param or else throw error
                ClassNode* curr_class = m_classtable->lookup(arg);

                bool foundInChildClass = false;
                while(strcmp(curr_class->name->spelling(), "TopClass") != 0 && curr_class->scope != NULL){
                  // cerr << curr_class->name->spelling() << endl;
                  if(strcmp(curr_class->name->spelling(), param) == 0 ){
                    foundInChildClass = true;
                    break;
                  }
                  curr_class = m_classtable->getParentOf(curr_class->name);
                }

                if(!foundInChildClass){
                  this->t_error(call_args_mismatch, p->m_attribute);
                }
              }
            } else if(argT != paramT){
              this->t_error(call_args_mismatch, p->m_attribute);          
            }
            param_i++;
          }
          // cerr << "Method call return: " << s->methodType.returnType.baseType << endl;
          p->m_attribute.m_type.baseType = s->methodType.returnType.baseType;
          // cerr << "Method Call RT: " << p->m_attribute.m_type.baseType << endl;
        }
      } else {
        // 24. Called Methods Must Exist on Receiver Object (error: no_class_method)
        this->t_error(no_class_method, p->m_attribute);
      }
    }
    
    void visitSelfCall(SelfCall *p) {
    
      //WRITE ME
      p->visit_children(this);
      // cerr << "self call" << endl;

      // 10. Identifiers which are used as method names must have the method type
      if(p->m_methodid->m_attribute.m_type.baseType != bt_function){
        // cerr << "aasdfsdfsdfaa" << endl;
        this->t_error(sym_type_mismatch, p->m_attribute);
      }

      list<Expression_ptr>::iterator exp_i;
      forall(exp_i, p->m_expression_list){
        if((*exp_i)->m_attribute.m_type.baseType == bt_function){
          // cerr << "asdf23wdf" << endl;
          this->t_error(sym_type_mismatch, p->m_attribute);       
        }
      }
      char* methodName = strdup(p->m_methodid->m_attribute.m_type.classType.classID);

      ClassNode* current_class = m_classtable->lookup(current_class_name);
      Symbol *s = current_class->scope->lookup(methodName);

      bool foundInParentClass = false;
      while(strcmp(current_class->name->spelling(), "TopClass") != 0 && current_class->scope != NULL){
        // cerr << current_class->name->spelling() << endl;
        s = current_class->scope->lookup(methodName);
        if(s != NULL){
          foundInParentClass = true;
          break;
        }
        current_class = m_classtable->getParentOf(current_class->name);
      }

      if(foundInParentClass){
        // cerr << p->m_expression_list->size() << " " << s->methodType.argsType.size() << endl;
        // 14. Number of Arguments Must Match Number of Parameters (error: call_narg_mismatch)
        if(s->methodType.argsType.size() != p->m_expression_list->size()){
          this->t_error(call_narg_mismatch, p->m_attribute);
        } else {
          // 15. Type of Arguments Must Match Type of Parameters (error: call_args_mismatch)
          list<Expression_ptr>::iterator exp_i2;
          vector<CompoundType>::iterator param_i;
          param_i = s->methodType.argsType.begin();
          forall(exp_i2, p->m_expression_list){
            Basetype argT = (*exp_i2)->m_attribute.m_type.baseType;
            Basetype paramT = param_i->baseType;

            if(argT == bt_object && paramT == bt_object){
              char* arg = strdup((*exp_i2)->m_attribute.m_type.classType.classID);
              char* param = strdup(param_i->classID);
              // cerr << arg << param << endl;
              if(strcmp(arg, param) != 0){
                // Loop through subclass to see if name matches param or else throw error
                ClassNode* curr_class = m_classtable->lookup(arg);

                bool foundInChildClass = false;
                while(strcmp(curr_class->name->spelling(), "TopClass") != 0 && current_class->scope != NULL){
                  // cerr << curr_class->name->spelling() << endl;
                  if(strcmp(curr_class->name->spelling(), param) == 0 ){
                    foundInChildClass = true;
                    break;
                  }
                  curr_class = m_classtable->getParentOf(curr_class->name);
                }

                if(!foundInChildClass){
                  this->t_error(call_args_mismatch, p->m_attribute);
                }
              }
            } else if(argT != paramT){
              this->t_error(call_args_mismatch, p->m_attribute);          
            }
            param_i++;
          }
          // cerr << "Method call return: " << s->methodType.returnType.baseType << endl;
          p->m_attribute.m_type.baseType = s->methodType.returnType.baseType;
        }
      } else {
        // 24. Called Methods Must Exist on Receiver Object (error: no_class_method)
        this->t_error(no_class_method, p->m_attribute);
      }
    }
    
    void visitVariable(Variable *p) {
      p->visit_children(this);

      char *varName = strdup(p->m_variableid->m_attribute.m_type.classType.classID);
      p->m_attribute.m_type.classType.classID = strdup(varName);

      // 9. No Usage of Undefined Variables (error: sym_name_undef)
      // TODO check parent classes
      Symbol *s;
      if(!m_symboltable->exist(varName)){
        char* className = strdup(current_class_name->spelling());
        // cerr << className << endl;

        ClassNode* current_class = m_classtable->lookup(className);

        bool foundInParentClass = false;
        // ClassNode
        while( strcmp(current_class->name->spelling(), "TopClass") != 0 && current_class->scope != NULL){
          s = current_class->scope->lookup(varName);
          if(s != NULL){
            foundInParentClass = true;
            break;
          }
          // m_classtable->getParentOf(current_class->name);
          current_class = m_classtable->getParentOf(current_class->name);
        }
        if(!foundInParentClass){
          // cerr << "ASDASDASD" << endl;
          t_error(sym_name_undef, p->m_attribute);
        }
      } else {
        s = m_symboltable->lookup(varName);  
      }

      
      p->m_attribute.m_type.baseType = s->baseType;
      p->m_attribute.m_type.classType.classID = s->classType.classID;
      // cerr << "visitVariable: " << varName << ", type: "<< bt_to_string(p->m_attribute.m_type.baseType) << endl;
    }
    
    // 23. Integer literals are of type Int and boolean literals are of type Bool.
    void visitIntegerLiteral(IntegerLiteral *p) {
      p->m_attribute.m_type.baseType = bt_integer;
    }
    
    void visitBooleanLiteral(BooleanLiteral *p) {
      p->m_attribute.m_type.baseType = bt_boolean;
    }
    
    void visitNothing(Nothing *p) {
      p->m_attribute.m_type.baseType = bt_nothing;
    }
    
    void visitSymName(SymName *p) {}
    
    void visitPrimitive(Primitive *p) {}
    
    void visitClassName(ClassName *p) {}
    
    
    void visitNullPointer() {}
};
