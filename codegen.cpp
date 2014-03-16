#include "ast.hpp"
#include "symtab.hpp"
#include "classhierarchy.hpp"
#include "primitive.hpp"
#include "assert.h"
#include <typeinfo>
#include <stdio.h>

#define forall(iterator,listptr) \
  for(iterator = listptr->begin(); iterator != listptr->end(); iterator++) \

#define forallr(riterator,listptr) \
  for(riterator = listptr->rbegin(); riterator != listptr->rend(); riterator++) \

class Codegen : public Visitor
{
  private:
  
  FILE * m_outputfile;
  SymTab *m_symboltable;
  ClassTable *m_classtable;
  
  const char * heapStart="_heap_start";
  const char * heapTop="_heap_top";
  const char * printFormat=".LC0";
  const char * printFun="Print";
  
  OffsetTable*currClassOffset;
  OffsetTable*currMethodOffset;

  char* currClassName;
  
  // basic size of a word (integers and booleans) in bytes
  static const int wordsize = 4;
  
  int label_count; //access with new_label
  
  // ********** Helper functions ********************************
  
  // this is used to get new unique labels (cleverly named label1, label2, ...)
  int new_label() { return label_count++; }

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

  // PART 1:
  // 1) get arithmetic expressions on integers working:
  //	  you wont really be able to run your code,
  //	  but you can visually inspect it to see that the correct
  //    chains of opcodes are being generated.
  // 2) get function calls working:
  //    if you want to see at least a very simple program compile
  //    and link successfully against gcc-produced code, you
  //    need to get at least this far
  // 3) get boolean operation working
  //    before we can implement any of the conditional control flow 
  //    stuff, we need to have booleans worked out.  
  // 4) control flow:
  //    we need a way to have if-elses and for loops in our language. 
  //
  // Hint: Symbols have an associated member variable called m_offset
  //    That offset can be used to figure out where in the activation 
  //    record you should look for a particuar variable
  
  ///////////////////////////////////////////////////////////////////////////////
  //
  //  function_prologue
  //  function_epilogue
  //
  //  Together these two functions implement the callee-side of the calling
  //  convention.  A stack frame has the following layout:
  //
  //                          <- SP (before pre-call / after epilogue)
  //  high -----------------
  //	   | actual arg 1  |
  //	   | ...           |
  //	   | actual arg n  |
  //	   -----------------
  //	   |  Return Addr  | 
  //	   =================
  //	   | temporary 1   |    <- SP (when starting prologue)
  //	   | ...           |
  //	   | temporary n   | 
  //  low -----------------   <- SP (when done prologue)
  //
  //
  //			  ||		
  //			  ||
  //			 \  /
  //			  \/
  //
  //
  //  The caller is responsible for placing the actual arguments
  //  and the return address on the stack. Actually, the return address
  //  is put automatically on the stack as part of the x86 call instruction.
  //
  //  On function entry, the callee
  //
  //  (1) allocates space for the callee's temporaries on the stack
  //  
  //  (2) saves callee-saved registers (see below) - including the previous activation record pointer (%ebp)
  //
  //  (3) makes the activation record pointer (frame pointer - %ebp) point to the start of the temporary region
  //
  //  (4) possibly copies the actual arguments into the temporary variables to allow easier access
  //
  //  On function exit, the callee:
  //
  //  (1) pops the callee's activation record (temporay area) off the stack
  //  
  //  (2) restores the callee-saved registers, including the activation record of the caller (%ebp)	 
  //
  //  (3) jumps to the return address (using the x86 "ret" instruction, this automatically pops the 
  //	  return address off the stack
  //
  //////////////////////////////////////////////////////////////////////////////
  //
  // Since we are interfacing with code produced by GCC, we have to respect the 
  // calling convention that GCC demands:
  //
  // Contract between caller and callee on x86: 
  //	 * after call instruction: 
  //		   o %eip points at first instruction of function 
  //		   o %esp+4 points at first argument 
  //		   o %esp points at return address 
  //	 * after ret instruction: 
  //		   o %eip contains return address 
  //		   o %esp points at arguments pushed by caller 
  //		   o called function may have trashed arguments 
  //		   o %eax contains return value (or trash if function is void) 
  //		   o %ecx, %edx may be trashed 
  //		   o %ebp, %ebx, %esi, %edi must contain contents from time of call 
  //	 * Terminology: 
  //		   o %eax, %ecx, %edx are "caller save" registers 
  //		   o %ebp, %ebx, %esi, %edi are "callee save" registers 
  ////////////////////////////////////////////////////////////////////////////////
  
  void init()
  {
    fprintf( m_outputfile, ".text\n\n");
    fprintf( m_outputfile, ".comm %s,4,4\n", heapStart);
    fprintf( m_outputfile, ".comm %s,4,4\n\n", heapTop);
    
    fprintf( m_outputfile, "%s:\n", printFormat);
    fprintf( m_outputfile, "       .string \"%%d\\n\"\n");
    fprintf( m_outputfile, "       .text\n");
    fprintf( m_outputfile, "       .globl  %s\n",printFun);
    fprintf( m_outputfile, "       .type   %s, @function\n\n",printFun);
    fprintf( m_outputfile, ".global %s\n",printFun);
    fprintf( m_outputfile, "%s:\n",printFun);
    fprintf( m_outputfile, "       pushl   %%ebp\n");
    fprintf( m_outputfile, "       movl    %%esp, %%ebp\n");
    fprintf( m_outputfile, "       movl    8(%%ebp), %%eax\n");
    fprintf( m_outputfile, "       pushl   %%eax\n");
    fprintf( m_outputfile, "       pushl   $.LC0\n");
    fprintf( m_outputfile, "       call    printf\n");
    fprintf( m_outputfile, "       addl    $8, %%esp\n");
    fprintf( m_outputfile, "       leave\n");
    fprintf( m_outputfile, "       ret\n\n");
  }

  void start(int programSize)
  {
    fprintf( m_outputfile, "# Start Function\n");
    fprintf( m_outputfile, ".global Start\n");
    fprintf( m_outputfile, "Start:\n");
    fprintf( m_outputfile, "        pushl   %%ebp\n");
    fprintf( m_outputfile, "        movl    %%esp, %%ebp\n");
    fprintf( m_outputfile, "        movl    8(%%ebp), %%ecx\n");
    fprintf( m_outputfile, "        movl    %%ecx, %s\n",heapStart);
    fprintf( m_outputfile, "        movl    %%ecx, %s\n",heapTop);
    fprintf( m_outputfile, "        addl    $%d, %s\n",programSize,heapTop);
    fprintf( m_outputfile, "        pushl   %s \n",heapStart);
    fprintf( m_outputfile, "        call    Program_start \n");
    fprintf( m_outputfile, "        leave\n");
    fprintf( m_outputfile, "        ret\n");
  }

  void allocSpace(int size)
  {
	// Optional WRITE ME
  }

////////////////////////////////////////////////////////////////////////////////
public:
  
  Codegen(FILE * outputfile, SymTab * st, ClassTable* ct)
  {
    m_outputfile = outputfile;
    m_symboltable = st;
    m_classtable = ct;
    label_count = 0;
    currMethodOffset=currClassOffset=NULL;
  }

  void visitProgramImpl(ProgramImpl *p) {

  	init();
    fprintf(m_outputfile, "# PROGRAM\n");

    p->visit_children(this);

  }
  void visitClassImpl(ClassImpl *p) {
    fprintf(m_outputfile, "## CLASS\n");

    ClassIDImpl* cid = ((ClassIDImpl*)p->m_classid_1);
    char* className = strdup(cid->m_classname->spelling());
    currClassName = strdup(className);

    // int offset = p->m_attribute.m_type.m_offset;
    // int size = p->m_attribute.m_type.m_size;

    CompoundType type;
    type.classID = strdup(className);
    type.baseType = bt_function;

    currClassOffset = new OffsetTable();
    // currClassOffset->insert(className, offset, size, type);

    // TODO: regular variables in class then objects in class
    int offset = 0;
    int size = 0;
    list<Declaration_ptr>::iterator dec_i;
    forall(dec_i, p->m_declaration_list){
      DeclarationImpl* d = (DeclarationImpl*)(*dec_i);

      d->m_type->accept(this);
      Basetype decl_type = d->m_type->m_attribute.m_type.baseType;

      list<VariableID_ptr>::iterator var_i;
      forall(var_i, d->m_variableid_list){
        VariableIDImpl* var = ((VariableIDImpl*)(*var_i));
        char *variableName = strdup(var->m_symname->spelling());

        cerr << "## Class: \'" << variableName << "\', type: " << bt_to_string(decl_type) << endl;

        CompoundType type;
        type.baseType = decl_type;
        if(type.baseType == bt_object){
          TObject* t = (TObject*)d->m_type;
          ClassIDImpl* cid = (ClassIDImpl*)t->m_classid;
          type.classID = strdup(cid->m_classname->spelling());

          // int objectSize = cid->m_attribute.m_type.m_size;
          int objectSize = m_classtable->lookup(type.classID)->offset->getTotalSize();
          cerr << "## object size: " << objectSize << ", classID: \"" << type.classID << "\"" << endl;
          // Do we allocate a pointer to the object within this object or
          // actually create this object         
          currClassOffset->insert(variableName, offset, objectSize, type);
          offset = (offset + objectSize);
          size = (size + objectSize);
        } else {
          currClassOffset->insert(variableName, offset, wordsize, type);
          size = (size + wordsize);
          offset = (offset + wordsize);
        }
      }
    }

    // Set the size
    cerr << "# CLASS SIZE: " << size << endl;
    currClassOffset->setTotalSize(size);
    p->m_attribute.m_type.m_size = size;

    // Store class info for others to reference
    ClassNode* currentClass = m_classtable->lookup(p->m_attribute.m_type.classType.classID);
    currClassOffset->copyTo(currentClass->offset);
    currClassOffset->setTotalSize(size);

    list<Method_ptr>::iterator meth_i;
    forall(meth_i, p->m_method_list){
      (*meth_i)->accept(this);
    }
  }
  void visitDeclarationImpl(DeclarationImpl *p) {
         // WRITEME

  }
  void visitMethodImpl(MethodImpl *p) {
    int programSize = 0;
    MethodIDImpl* m = ((MethodIDImpl*)p->m_methodid);
    visitMethodIDImpl(m);
    char* methodName = strdup(m->m_symname->spelling());

    MethodBody* mb = ((MethodBodyImpl*)p->m_methodbody);
    MethodBodyImpl* mbi = ((MethodBodyImpl*)mb);
    int num_args = p->m_parameter_list->size();
    int num_locals = mbi->m_declaration_list->size();
    int totalSize = num_locals*4;

    // int offset = p->m_attribute.m_type.m_offset;
    // p->m_attribute.m_type.m_offset = num_locals;
    // p->m_attribute.m_type.m_size = totalSize;

    currMethodOffset = new OffsetTable();
    // currMethodOffset->insert(methodName, offset, size, type);
    currMethodOffset->setTotalSize(totalSize);
    currMethodOffset->setParamSize(num_args*wordsize);

    int offset = wordsize*2;
    list<Parameter_ptr>::iterator par_i;
    forall(par_i, p->m_parameter_list){
      ParameterImpl* pa = (ParameterImpl*)(*par_i);
      char* paramName = strdup(((VariableIDImpl*)(pa->m_variableid))->m_symname->spelling());

      pa->m_type->accept(this);
      Basetype param_type = pa->m_type->m_attribute.m_type.baseType;
      
      CompoundType type;
      type.baseType = param_type;
      if(type.baseType == bt_object){
        TObject* t = (TObject*)pa->m_type;
        ClassIDImpl* cid = (ClassIDImpl*)t->m_classid;
        type.classID = strdup(cid->m_classname->spelling());
      }

      cerr << "## Param: \'" << paramName << "\', type: " << bt_to_string(param_type) << ", offset: " << offset << endl;
      currMethodOffset->insert(paramName, offset, wordsize, type);
      programSize += wordsize;
      offset = offset + wordsize;
    }

    fprintf(m_outputfile, "### METHOD\n");

    fprintf(m_outputfile, "%s_%s:\n", currClassName, methodName);
    // PROLOGUE
    cerr << "## prologue" << endl;
    // save the activation record pointer of the caller function
    fprintf(m_outputfile, "        pushl %%ebp\n");
    // setup activation record pointer
    fprintf(m_outputfile, "        movl %%esp, %%ebp\n");

    // allocate space for local variables
    // Subtract from stack pointer
    fprintf(m_outputfile,"        subl $%d,%%esp\n", currMethodOffset->getTotalSize());

    p->visit_children(this);

    // loop through and get sizes of all symbols then we can start
    if( strcmp(methodName, "start") == 0 ) start(programSize);

  }
  void visitMethodBodyImpl(MethodBodyImpl *p) {
    fprintf(m_outputfile, "#### METHODBODY\n");

    int offset = (-wordsize);
    list<Declaration_ptr>::iterator dec_i;
    forall(dec_i, p->m_declaration_list){
      DeclarationImpl* d = (DeclarationImpl*)(*dec_i);

      d->m_type->accept(this);
      Basetype decl_type = d->m_type->m_attribute.m_type.baseType;

      list<VariableID_ptr>::iterator var_i;
      forall(var_i, d->m_variableid_list){
        VariableIDImpl* var = ((VariableIDImpl*)(*var_i));
        char *variableName = strdup(var->m_symname->spelling());

        CompoundType type;
        type.baseType = decl_type;
        if(type.baseType == bt_object){
          TObject* t = (TObject*)d->m_type;
          ClassIDImpl* cid = (ClassIDImpl*)t->m_classid;
          type.classID = strdup(cid->m_classname->spelling());
          int objectSize = cid->m_attribute.m_type.m_size;

          // fprintf(m_outputfile, "movl _heap_top, %%ecx\n");
          // fprintf(m_outputfile, "addl $%d, _heap_top\n", objectSize);
        }

        cerr << "## Local: \'" << variableName << "\', type: " << bt_to_string(decl_type) << endl; //", classID: " << type.classID << endl;
        currMethodOffset->insert(variableName, offset, wordsize, type);
        offset = (offset - wordsize);
      }
    }

    p->visit_children(this);
  }
  void visitParameterImpl(ParameterImpl *p) {}
  void visitAssignment(Assignment *p) {
    fprintf(m_outputfile, "##### ASSIGNMENT\n");
    p->visit_children(this);

    char* variableName = strdup(((VariableIDImpl*)(p->m_variableid))->m_symname->spelling());
    int offset = currMethodOffset->get_offset(variableName);

    fprintf(m_outputfile, "        popl %%eax\n");
    fprintf(m_outputfile, "        movl %%eax, %d(%%ebp)\n", offset);

  }
  void visitIf(If *p) {
    fprintf(m_outputfile, "##### IF ELSE\n");

    p->m_expression->accept(this);
    int label = new_label();
    fprintf(m_outputfile, "        popl %%eax\n");
    fprintf(m_outputfile, "        cmp $1, %%eax\n");
    fprintf(m_outputfile, "        jne end%d\n", label);
    p->m_statement->accept(this);
    fprintf(m_outputfile, "end%d:\n", label);

  }
  void visitPrint(Print *p) {
    fprintf(m_outputfile, "##### PRINT\n");
    p->visit_children(this);

    // TODO: acts kind of weird. does it though?
    fprintf(m_outputfile, "        call Print\n");
  }
  void visitReturnImpl(ReturnImpl *p) {
    p->visit_children(this);
    // Store the return value
    // fprintf( m_outputfile, "  call Print\n");
    fprintf(m_outputfile, "        popl %%eax\n");

    // EPILOGUE
    cerr << "## epilogue" << endl;
    // clean up  activation record
    // deallocating the local variable space allocated
    fprintf(m_outputfile, "        addl $%d, %%esp\n", currMethodOffset->getTotalSize());
    // restoring the caller's activation record pointer
    fprintf(m_outputfile, "        leave\n");
    // returning to the return address
    fprintf(m_outputfile, "        ret\n\n");
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
  }
  void visitClassIDImpl(ClassIDImpl *p) {
    // fprintf(m_outputfile, "####### CLASS ID\n");
  }
  void visitVariableIDImpl(VariableIDImpl *p) {
    // fprintf(m_outputfile, "####### VARIABLE ID\n");
  }
  void visitMethodIDImpl(MethodIDImpl *p) {
    // fprintf(m_outputfile, "####### METHOD ID\n");
  }
  void visitPlus(Plus *p) {
    fprintf(m_outputfile, "####### ADD\n");

    p->visit_children(this);

    fprintf(m_outputfile, "        popl %%ebx\n");
    fprintf(m_outputfile, "        popl %%eax\n");
    fprintf(m_outputfile, "        addl %%ebx, %%eax\n");
    fprintf(m_outputfile, "        pushl %%eax\n");

  }
  void visitMinus(Minus *p) {
    fprintf(m_outputfile, "###### MINUS\n");

    p->visit_children(this);

    fprintf(m_outputfile, "        popl %%ebx\n");
    fprintf(m_outputfile, "        popl %%eax\n");
    fprintf(m_outputfile, "        subl %%ebx, %%eax\n");
    fprintf(m_outputfile, "        pushl %%eax\n");

  }
  void visitTimes(Times *p) {
    fprintf(m_outputfile, "###### TIMES\n");

    p->visit_children(this);
    
    fprintf(m_outputfile, "        popl %%ebx\n");
    fprintf(m_outputfile, "        popl %%eax\n");
    fprintf(m_outputfile, "        imull %%ebx, %%eax\n");
    fprintf(m_outputfile, "        pushl %%eax\n");

  }
  void visitDivide(Divide *p) {
    fprintf(m_outputfile, "###### DIVIDE\n");

    p->visit_children(this);
    
    fprintf(m_outputfile, "        popl %%ebx\n");
    fprintf(m_outputfile, "        popl %%eax\n");
    fprintf(m_outputfile, "        cdq\n");
    fprintf(m_outputfile, "        idivl %%ebx\n");
    fprintf(m_outputfile, "        pushl %%eax\n");

  }
  void visitAnd(And *p) {
    fprintf(m_outputfile, "###### AND\n");

    p->visit_children(this);
    
    fprintf(m_outputfile, "        popl %%ebx\n");
    fprintf(m_outputfile, "        popl %%eax\n");
    fprintf(m_outputfile, "        andl %%ebx, %%eax\n");
    fprintf(m_outputfile, "        pushl %%eax\n");

  }
  void visitLessThan(LessThan *p) {
    p->visit_children(this);

    int l = new_label();

    fprintf(m_outputfile, "###### LessThan\n");

    fprintf(m_outputfile, "        popl %%ebx\n");
    fprintf(m_outputfile, "        popl %%eax\n");
    fprintf(m_outputfile, "        cmp %%ebx, %%eax\n");
    fprintf(m_outputfile, "        jl equal%d\n", l);

    fprintf(m_outputfile, "        pushl $0\n");
    fprintf(m_outputfile, "        jmp end%d\n", l);

    fprintf(m_outputfile, "equal%d:\n", l);
    fprintf(m_outputfile, "        pushl $1\n");

    fprintf(m_outputfile, "end%d:\n", l);

  }
  void visitLessThanEqualTo(LessThanEqualTo *p) {
    p->visit_children(this);

    int l = new_label();
    fprintf(m_outputfile, "###### LessThanEqualTo\n");

    fprintf(m_outputfile, "        popl %%ebx\n");
    fprintf(m_outputfile, "        popl %%eax\n");
    fprintf(m_outputfile, "        cmp %%ebx, %%eax\n");
    fprintf(m_outputfile, "        jle equal%d\n", l);

    fprintf(m_outputfile, "        pushl $0\n");
    fprintf(m_outputfile, "        jmp end%d\n", l);

    fprintf(m_outputfile, "equal%d:\n", l);
    fprintf(m_outputfile, "        pushl $1\n");

    fprintf(m_outputfile, "end%d:\n", l);

  }
  void visitNot(Not *p) {
    fprintf(m_outputfile, "###### NOT\n");

    p->visit_children(this);
    
    fprintf(m_outputfile, "        popl %%eax\n");
    fprintf(m_outputfile, "        not %%eax\n");
    fprintf(m_outputfile, "        pushl %%eax\n");

  }
  void visitUnaryMinus(UnaryMinus *p) {
    fprintf(m_outputfile, "###### AND\n");

    p->visit_children(this);
    
    fprintf(m_outputfile, "        popl %%eax\n");
    fprintf(m_outputfile, "        negl %%eax\n");
    fprintf(m_outputfile, "        pushl %%eax\n");

  }
  void visitMethodCall(MethodCall *p) {
    // TODO: check super classes as well
    // Push arguments on the stack
    int param_size = p->m_expression_list->size();
    list<Expression_ptr>::reverse_iterator exp_i;
    forallr(exp_i, p->m_expression_list){
      cerr << "# parameter" << endl;
      (*exp_i)->accept(this);
    }

    char* variableName = strdup(((VariableIDImpl*)(p->m_variableid))->m_symname->spelling());
    CompoundType type = currMethodOffset->get_type(variableName);
    cerr << "# MethodCall, class name: " << type.classID << ", type: " << bt_to_string(type.baseType) << endl;

    // Push return address
    // Call the function
    MethodIDImpl* m = ((MethodIDImpl*)p->m_methodid);
    visitMethodIDImpl(m);
    char* methodName = strdup(m->m_symname->spelling());
    fprintf(m_outputfile, "        call %s_%s\n", type.classID, methodName);

    // POST-CALL
    cerr << "## post-call" << endl;
    fprintf(m_outputfile, "        addl $%d, %%ebp\n", param_size*wordsize);

    // Put return value onto stack
    fprintf(m_outputfile, "        pushl %%eax\n");

  }
  void visitSelfCall(SelfCall *p) {

         // WRITEME
    // PRE-CALL
    cerr << "## pre-call" << endl;
    // Push arguments on the stack
    int param_size = p->m_expression_list->size();
    list<Expression_ptr>::reverse_iterator exp_i;
    forallr(exp_i, p->m_expression_list){
      cerr << "# parameter" << endl;
      (*exp_i)->accept(this);
    }

    // Push return address
    // fprintf(m_outputfile, "        pushl %%esp\n");

    // Call the function
    MethodIDImpl* m = ((MethodIDImpl*)p->m_methodid);
    visitMethodIDImpl(m);
    char* methodName = strdup(m->m_symname->spelling());
    fprintf(m_outputfile, "        call %s_%s\n", currClassName, methodName);

    // POST-CALL
    cerr << "## post-call" << endl;
    fprintf(m_outputfile, "        addl $%d, %%ebp\n", param_size*wordsize);

    // Put return value onto stack
    fprintf(m_outputfile, "        pushl %%eax\n");

  }
  void visitVariable(Variable *p) {

         // WRITEME
    char* variableName = strdup(((VariableIDImpl*)(p->m_variableid))->m_symname->spelling());
    int offset = currMethodOffset->get_offset(variableName);
    int size = currMethodOffset->get_size(variableName);
    cerr << "# variable: " << variableName << ", offset: " << offset << ", size: " << size << endl;
    fprintf(m_outputfile, "        pushl %d(%%ebp)\n", offset);
    // -4(%ebp)

  }
  void visitIntegerLiteral(IntegerLiteral *p) {
    fprintf(m_outputfile, "####### INT literal\n");
    fprintf(m_outputfile, "        pushl $%d\n", p->m_primitive->m_data);

  }
  void visitBooleanLiteral(BooleanLiteral *p) {
    fprintf(m_outputfile, "####### BOOL literal\n");
    fprintf(m_outputfile, "        pushl $%d\n", p->m_primitive->m_data);

  }
  void visitNothing(Nothing *p) {

         // WRITEME

  }
  void visitSymName(SymName *p) {

         // WRITEME

  }
  void visitPrimitive(Primitive *p) {

         // WRITEME

  }
  void visitClassName(ClassName *p) {

         // WRITEME

  }

  void visitNullPointer() {}
 
};




