#include "ast.hpp"
#include "symtab.hpp"
#include "classhierarchy.hpp"
#include "primitive.hpp"
#include "assert.h"
#include <typeinfo>
#include <stdio.h>

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
  
  // basic size of a word (integers and booleans) in bytes
  static const int wordsize = 4;
  
  int label_count; //access with new_label
  
  // ********** Helper functions ********************************
  
  // this is used to get new unique labels (cleverly named label1, label2, ...)
  int new_label() { return label_count++; }

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
         // WRITEME
  p->visit_children(this);

  }
  void visitClassImpl(ClassImpl *p) {

         // WRITEME

  }
  void visitDeclarationImpl(DeclarationImpl *p) {

         // WRITEME

  }
  void visitMethodImpl(MethodImpl *p) {

         // WRITEME

  }
  void visitMethodBodyImpl(MethodBodyImpl *p) {

         // WRITEME

  }
  void visitParameterImpl(ParameterImpl *p) {
		
         // WRITEME

  }
  void visitAssignment(Assignment *p) {

         // WRITEME

  }
  void visitIf(If *p) {

         // WRITEME

  }
  void visitPrint(Print *p) {
	
         // WRITEME

  }
  void visitReturnImpl(ReturnImpl *p) {

         // WRITEME

  }
  void visitTInteger(TInteger *p) {}
  void visitTBoolean(TBoolean *p) {}
  void visitTNothing(TNothing *p) {}
  void visitTObject(TObject *p) {

         // WRITEME

  }
  void visitClassIDImpl(ClassIDImpl *p) {

         // WRITEME

  }
  void visitVariableIDImpl(VariableIDImpl *p) {

         // WRITEME

  }
  void visitMethodIDImpl(MethodIDImpl *p) {

         // WRITEME

  }
  void visitPlus(Plus *p) {

         // WRITEME

  }
  void visitMinus(Minus *p) {

         // WRITEME

  }
  void visitTimes(Times *p) {

         // WRITEME

  }
  void visitDivide(Divide *p) {

         // WRITEME

  }
  void visitAnd(And *p) {

         // WRITEME

  }
  void visitLessThan(LessThan *p) {

         // WRITEME

  }
  void visitLessThanEqualTo(LessThanEqualTo *p) {

         // WRITEME

  }
  void visitNot(Not *p) {

         // WRITEME

  }
  void visitUnaryMinus(UnaryMinus *p) {

         // WRITEME

  }
  void visitMethodCall(MethodCall *p) {

         // WRITEME

  }
  void visitSelfCall(SelfCall *p) {

         // WRITEME

  }
  void visitVariable(Variable *p) {

         // WRITEME

  }
  void visitIntegerLiteral(IntegerLiteral *p) {

         // WRITEME

  }
  void visitBooleanLiteral(BooleanLiteral *p) {

         // WRITEME

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




