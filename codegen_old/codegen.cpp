#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "assert.h"
#include <typeinfo>
#include <stdio.h>

class Codegen : public Visitor
{
	private:

	FILE * m_outputfile;
	SymTab *m_st;

	// basic size of a word (integers and booleans) in bytes
	static const int wordsize = 4;

	int label_count; //access with new_label

	// ********** Helper functions ********************************

	// this is used to get new unique labels (cleverly named label1, label2, ...)
	int new_label() { return label_count++; }

	// this mode is used for the code
	void set_text_mode() { fprintf( m_outputfile, ".text\n\n"); }
	
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
	//                          <- SP (before pre-call / after post-ret)
	//  high -----------------
	//       | actual arg n  |
	//       | ...           |
	//       | actual arg 1  |  <- SP (just before call / just after ret)
	//       -----------------
	//       |  Return Addr  |  <- SP (just after call / just before ret)
	//       =================
	//       | previous %ebp |
	//       -----------------
	//       | temporary 1   |
	//       | ...           |
	//       | temporary n   |  <- SP (after prologue / before epilogue)
	//  low  -----------------
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
	//  (1) pops the callee's activation record (temporary area) off the stack
	//  
	//  (2) restores the callee-saved registers, including the activation record of the caller (%ebp)	 
	//
	//  (3) jumps to the return address (using the x86 "ret" instruction, this automatically pops the 
	//	  return address of the stack. After the ret, remove the arguments from the stack
	//
	//	For more info on this convention, see http://unixwiz.net/techtips/win32-callconv-asm.html
	//
	//	This convention is called __cdecl
	//
	//////////////////////////////////////////////////////////////////////////////
  
  void emit_prologue(SymName *name, unsigned int size_locals, unsigned int num_args)
  {
    // WRITEME
  }

  void emit_epilogue()
  {
    // WRITEME
  }
  
  // HERE: more functions to emit code
  //check if main func or other
  int isMain;
  void inMain(bool checkMain){
    if(checkMain == true)
        isMain = 1;
    else
        isMain=0;
  }

////////////////////////////////////////////////////////////////////////////////

public:
  
  Codegen(FILE * outputfile, SymTab * st)
  {
	m_outputfile = outputfile;
	m_st = st;
	
	label_count = 0;
  }
  
  void visitProgram(Program * p)
  {
    set_text_mode();
    p -> visit_children(this);
  }
  void visitFunc(Func * p)
  {
    // WRITEME
    char *name = strdup(p -> m_symname -> spelling());
    if ( strcmp(name,"Main") == 0 ) {
                inMain(true);
                fprintf( m_outputfile, ".globl Main\n");
                fprintf( m_outputfile, "Main:\n");
                fprintf(m_outputfile, "push %%ebp\n");
                fprintf(m_outputfile, "mov %%esp, %%ebp\n");
                //sub esp, 4 Make room for 4-byte local variables
                fprintf(m_outputfile,"sub $%i,%%esp\n",m_st->scopesize(p->m_function_block->m_attribute.m_scope));

                p->visit_children(this);
                fprintf(m_outputfile, "mov %%ebp, %%esp \n");
                fprintf(m_outputfile, "pop %%ebp\n");
                fprintf( m_outputfile, "\tret\n");
                inMain(false);
                return;
    }
    else{
        //prologue
        fprintf(m_outputfile, "####INSIDE FUNCTION\n");
        fprintf(m_outputfile, "_%s: \n",  name);
        fprintf(m_outputfile, "push %%ebp\n");
        fprintf(m_outputfile, "mov %%esp, %%ebp\n");
        //sub esp, 4   ; Make room for one 4-byte local variable.
        fprintf(m_outputfile,"sub $%i,%%esp\n",m_st->scopesize(p->m_function_block->m_attribute.m_scope));
//        fprintf(m_outputfile, "pop %%ebx\n");
//        list<Param_ptr>::iterator iter;
//        for (iter = p->m_param_list->begin(); iter != p->m_param_list->end(); iter++) {
//            Symbol *s=m_st->lookup((*iter)->m_attribute.m_scope,strdup((*iter)->m_symname->spelling()));
//            int offset=4+s->get_offset();
//            fprintf(m_outputfile, "popl %%eax\n");
//            fprintf(m_outputfile, "mov %%eax, -%d(%%ebp)\n", offset);
//        }
//        fprintf(m_outputfile, "push %%ebx\n");
        p->visit_children(this);
        //epilogue
        fprintf(m_outputfile, "mov %%ebp, %%esp \n");
        fprintf(m_outputfile, "pop %%ebp\n");
        fprintf(m_outputfile, "\tret\n\n"); //could return size of local vars
        return;
    }


  }
  void visitFunction_block(Function_block * p)
  {
    // WRITEME
    fprintf(m_outputfile, "#### visitFuncBlock\n");
//    fprintf(m_outputfile,"sub $%i,%%esp\n",m_st->scopesize(p->m_attribute.m_scope));
//    list<Decl_ptr>::iterator iter;
//    list<SymName_ptr>::iterator iter2;
//    for(iter = p->m_decl_list->begin(); iter != p->m_decl_list->end(); iter++){
//       for(iter2=(*iter)->m_symname_list->begin(); iter2!=(*iter)->m_symname_list->end(); iter2++)
////           fprintf(m_outputfile,"hello %s\n", strdup((*iter2)->spelling()));
//    }
    p->visit_children(this);
  }
  void visitNested_block(Nested_block * p)
  {
    // WRITEME
       p->visit_children(this);
  }
  void visitAssignment(Assignment * p)
  {
    p->visit_children(this);
    fprintf( m_outputfile, "#### ASSIGN\n");
    Symbol *s=m_st->lookup(p->m_attribute.m_scope,strdup(p->m_symname->spelling()));
    int offset=4+s->get_offset();
    fprintf( m_outputfile, "popl %%eax\n", offset);
    fprintf( m_outputfile, "mov %%eax, -%d(%%ebp)\n", offset);
  }
  void visitArrayAssignment(ArrayAssignment * p)
  {
    p->m_expr_2->accept(this);
    fprintf( m_outputfile, "#### ARRAY ASSIGN\n");
    Symbol *s=m_st->lookup(p->m_attribute.m_scope,strdup(p->m_symname->spelling()));
    int offset=s->get_offset()+4*p->m_expr_1->m_attribute.m_lattice_elem.value;
    fprintf( m_outputfile, "popl %%eax\n", offset);
    fprintf( m_outputfile, "mov %%eax, -%d(%%ebp)\n", offset);

  }
  void visitCall(Call * p)
  {
        fprintf(m_outputfile, "####VISIT CALL\n");
        if(p -> m_attribute.m_lattice_elem == TOP)
             p -> visit_children(this);
        list<Expr_ptr>::reverse_iterator iter = p->m_expr_list->rbegin();
         for(; iter != p->m_expr_list->rend(); ++iter){
             (*iter)->accept(this);
//             fprintf(m_outputfile, "pushl $%d\n", (*iter)->m_attribute.m_lattice_elem.value);
         }
       
        char *name = strdup(p -> m_symname_2 -> spelling());
        //call func
       
        fprintf(m_outputfile, "call _%s\n", name);
        //add esp, offset
        //result stored in %eax
        Symbol *s=m_st->lookup(p->m_attribute.m_scope,strdup(p->m_symname_1->spelling()));
        int offset=4+s->get_offset();
        int offset2=m_st->scopesize(p->m_attribute.m_scope);
        //we have to see if it is a local var or arg
        fprintf(m_outputfile, "movl %%eax, -%d(%%ebp)\n",offset);
  }
  void visitArrayCall(ArrayCall *p)
  {
      fprintf(m_outputfile, "####VISIT ARRAY CALL\n");
        if(p -> m_attribute.m_lattice_elem == TOP)
             p -> visit_children(this);
        list<Expr_ptr>::reverse_iterator iter = p->m_expr_list_2->rbegin();
         for(; iter != p->m_expr_list_2->rend(); ++iter){
             (*iter)->accept(this);
//             fprintf(m_outputfile, "pushl $%d\n", (*iter)->m_attribute.m_lattice_elem.value);
         }
       
        char *name = strdup(p -> m_symname_2 -> spelling());
        //call func
       
        fprintf(m_outputfile, "call _%s\n", name);
        //add esp, offset
        //result stored in %eax
        Symbol *s=m_st->lookup(p->m_attribute.m_scope,strdup(p->m_symname_1->spelling()));
        int offset=s->get_offset()+4*p->m_expr_1->m_attribute.m_lattice_elem.value;
        int offset2=m_st->scopesize(p->m_attribute.m_scope);
        //we have to see if it is a local var or arg
        fprintf(m_outputfile, "movl %%eax, -%d(%%ebp)\n",offset);
  }
  void visitReturn(Return * p)
  {
    p -> visit_children(this);
    fprintf( m_outputfile, "#### RETURN\n");
    fprintf( m_outputfile, "popl %%eax\n");
    

  }

  // control flow
  void visitIfNoElse(IfNoElse * p)
  {// WRITEME
       fprintf( m_outputfile, "#### IFNOELSE\n");
       if (p ->m_expr->m_attribute.m_lattice_elem != TOP ) {
           if( p->m_expr->m_attribute.m_lattice_elem.value == 1)
                p->m_nested_block->visit_children(this);
          return;
       }
      
      int label=new_label();
      p ->m_expr->visit_children(this);
      fprintf( m_outputfile, "popl %%eax\n");
      fprintf( m_outputfile, "movl $0, %%ebx\n");
      fprintf( m_outputfile, "cmp %%eax, %%ebx\n");
      fprintf( m_outputfile, "je skip_if_%i\n",label);
      p->m_nested_block->visit_children(this);
      fprintf( m_outputfile, "skip_if_%i:\n",label);
    
  }
  void visitIfWithElse(IfWithElse * p)
  {
    fprintf( m_outputfile, "#### IF WITH ELSE\n");
       if (p ->m_expr->m_attribute.m_lattice_elem != TOP ) {
           if( p->m_expr->m_attribute.m_lattice_elem.value == 1)
                p->m_nested_block_1->visit_children(this);
           else
               p->m_nested_block_2->visit_children(this);
          return;
       }
    
      int label=new_label();
      p ->m_expr->visit_children(this);
      fprintf( m_outputfile, "#### IFWITHELSE\n");
      fprintf( m_outputfile, "popl %%eax\n");
      fprintf( m_outputfile, "movl $0, %%ebx\n");
      fprintf( m_outputfile, "cmp %%eax, %%ebx\n");
      fprintf( m_outputfile, "je skip_if_%i\n",label);
      p->m_nested_block_1->visit_children(this);
      fprintf( m_outputfile, "skip_if_%i:",label);
      p->m_nested_block_1->visit_children(this);

  }
  void visitForLoop(ForLoop * p)
  {
    fprintf( m_outputfile, "#### VISIT FOR LOOP\n");
    p->m_stat_1->accept(this);
    int label=new_label();
    fprintf( m_outputfile, "jmp compare_expr%i\n",label);
    fprintf( m_outputfile, "func_body%i:\n",label);
    p->m_nested_block->accept(this);
    p->m_stat_2->accept(this);
    fprintf( m_outputfile, "compare_expr%i:\n",label);
    p->m_expr->accept(this);
    fprintf( m_outputfile, "pop %%ebx\n");
    fprintf( m_outputfile, "mov $1, %%eax\n");
    fprintf( m_outputfile, "cmp %%ebx,%%eax\n");
    fprintf( m_outputfile, "je func_body%i\n",label);
    
     fprintf( m_outputfile, "#### END FOR LOOP\n");
  }

  
  void visitNone(None *p)
  {
    // Nothing is emitted
  }
  
  // variable declarations (no code generation needed)
  void visitDecl(Decl * p)
  {
     p -> visit_children(this);
//     fprintf( m_outputfile, "#### VISIT Decl\n");
     int totalLocalVars=0;
     list<SymName_ptr>::iterator iter;
     for(iter = p->m_symname_list->begin(); iter != p->m_symname_list->end(); iter++){
         totalLocalVars++;
     }
    
  }
  void visitParam(Param *p)
  {
      fprintf(m_outputfile, "#### Param Visited\n");
      p->visit_children(this);
            Symbol *s=m_st->lookup(p->m_attribute.m_scope,strdup(p->m_symname->spelling()));
            int offset=4+s->get_offset();
            int offset2=8+s->get_offset();
            fprintf(m_outputfile, "mov %d(%%ebp), %%eax\n",offset2);
            fprintf(m_outputfile, "mov %%eax, -%d(%%ebp)\n", offset);
  }
  
  // types (no code generation needed)
  void visitTInteger(TInteger * p)
  {
  }
  void visitTBoolean(TBoolean * p)
  {
  }
  void visitTIntArray(TIntArray * p)
  {
  }

  // comparison operations
  void visitCompare(Compare * p)
  {
    fprintf( m_outputfile, "#### Compare ==\n");
    // WRITEME
//     int label=new_label();
//      if (p -> m_attribute.m_lattice_elem != TOP) {
//          fprintf( m_outputfile, " pushl $%d\n", p->m_attribute.m_lattice_elem.value);
//          return;
//      }
    int lbl=new_label();
      p -> visit_children(this);
      fprintf( m_outputfile, "popl %%ebx\n");
      fprintf( m_outputfile, "popl %%eax\n");
      fprintf( m_outputfile, "cmp %%ebx,%%eax\n");
      fprintf(m_outputfile, "je equal%d\n", lbl);
      
      fprintf(m_outputfile, "pushl $0\n");
      fprintf(m_outputfile, "jmp end%d\n", lbl);
      
      fprintf(m_outputfile, "equal%d:\n", lbl);
      fprintf(m_outputfile, "pushl $1\n");
      
      fprintf(m_outputfile, "end%d:\n", lbl);
      
      
      
  }
  void visitNoteq(Noteq * p)
  {
    // WRITEME
//      fprintf( m_outputfile, "#### NOTEQ !=\n");
//      int label=new_label();
//      if (p -> m_attribute.m_lattice_elem != TOP) {
//          fprintf( m_outputfile, " pushl $%d\n", p->m_attribute.m_lattice_elem.value);
//          return;
//      }
      p -> visit_children(this);
      fprintf( m_outputfile, "popl %%ebx\n");
      fprintf( m_outputfile, "popl %%eax\n");
      fprintf( m_outputfile, "cmp %%ebx,%%eax\n");
      int lbl=new_label();
      fprintf(m_outputfile, "jne equal%d\n", lbl);
      
    fprintf(m_outputfile, "pushl $0\n");
      fprintf(m_outputfile, "jmp end%d\n", lbl);
      
      fprintf(m_outputfile, "equal%d:\n", lbl);
      fprintf(m_outputfile, "pushl $1\n");
      
      fprintf(m_outputfile, "end%d:\n", lbl);
  }
  void visitGt(Gt * p)
  {
    // WRITEME
//     fprintf( m_outputfile, "#### GREATER Than\n");
//      int label=new_label();
//      if (p -> m_attribute.m_lattice_elem != TOP) {
//          fprintf( m_outputfile, " pushl $%d\n", p->m_attribute.m_lattice_elem.value);
//          return;
//      }
      p -> visit_children(this);
      int lbl=new_label();
       fprintf( m_outputfile, "popl %%ebx\n");
      fprintf( m_outputfile, "popl %%eax\n");
      fprintf( m_outputfile, "cmp %%ebx,%%eax\n");
      fprintf(m_outputfile, "jne equal%d\n", lbl);
      
     fprintf(m_outputfile, "pushl $0\n");
      fprintf(m_outputfile, "jmp end%d\n", lbl);
      
      fprintf(m_outputfile, "equal%d:\n", lbl);
      fprintf(m_outputfile, "pushl $1\n");
      
      fprintf(m_outputfile, "end%d:\n", lbl);

  }
  void visitGteq(Gteq * p)
  {
    // WRITEME
//                  int label=new_label();
//      if (p -> m_attribute.m_lattice_elem != TOP) {
//          fprintf( m_outputfile, " pushl $%d\n", p->m_attribute.m_lattice_elem.value);
//          return;
//      }
      p -> visit_children(this);
   int lbl=new_label();
    fprintf( m_outputfile, "popl %%ebx\n");
      fprintf( m_outputfile, "popl %%eax\n");
      fprintf( m_outputfile, "cmp %%ebx,%%eax\n");
      fprintf(m_outputfile, "jge equal%d\n", lbl);
      
      fprintf(m_outputfile, "pushl $0\n");
      fprintf(m_outputfile, "jmp end%d\n", lbl);
      
      fprintf(m_outputfile, "equal%d:\n", lbl);
      fprintf(m_outputfile, "pushl $1\n");
      
      fprintf(m_outputfile, "end%d:\n", lbl);

  }
  void visitLt(Lt * p)
  {
    // WRITEME
//             int label=new_label();
//      if (p -> m_attribute.m_lattice_elem != TOP) {
//          fprintf( m_outputfile, " pushl $%d\n", p->m_attribute.m_lattice_elem.value);
//          return;
//      }
      p -> visit_children(this);
      int lbl=new_label();
       fprintf( m_outputfile, "popl %%ebx\n");
      fprintf( m_outputfile, "popl %%eax\n");
      fprintf( m_outputfile, "cmp %%ebx,%%eax\n");
      fprintf(m_outputfile, "jl equal%d\n", lbl);
      
     fprintf(m_outputfile, "pushl $0\n");
      fprintf(m_outputfile, "jmp end%d\n", lbl);
      
      fprintf(m_outputfile, "equal%d:\n", lbl);
      fprintf(m_outputfile, "pushl $1\n");
      
      fprintf(m_outputfile, "end%d:\n", lbl);

  }
  void visitLteq(Lteq * p)
  {
    // WRITEME
//                  int label=new_label();
//      if (p -> m_attribute.m_lattice_elem != TOP) {
//          fprintf( m_outputfile, " pushl $%d\n", p->m_attribute.m_lattice_elem.value);
//          return;
//      }
      p -> visit_children(this);
 int lbl=new_label();
      fprintf( m_outputfile, "popl %%ebx\n");
      fprintf( m_outputfile, "popl %%eax\n");
      fprintf( m_outputfile, "cmp %%ebx,%%eax\n");
      fprintf(m_outputfile, "jle equal%d\n", lbl);
      
     fprintf(m_outputfile, "pushl $0\n");
      fprintf(m_outputfile, "jmp end%d\n", lbl);
      
      fprintf(m_outputfile, "equal%d:\n", lbl);
      fprintf(m_outputfile, "pushl $1\n");
      
      fprintf(m_outputfile, "end%d:\n", lbl);
  }

  // arithmetic and logic operations
  void visitAnd(And * p)
  {
     fprintf( m_outputfile, "#### AND\n");
     if (p -> m_attribute.m_lattice_elem != TOP) {
         fprintf( m_outputfile, " pushl $%d\n", p -> m_attribute.m_lattice_elem.value);
         return;
     }

     p -> visit_children(this);

     fprintf( m_outputfile, " popl %%ebx\n");
     fprintf( m_outputfile, " popl %%eax\n");
     fprintf( m_outputfile, " andl %%ebx, %%eax\n");
     fprintf( m_outputfile, " pushl %%eax\n");
  }
  void visitOr(Or * p)
  {
    fprintf( m_outputfile, "#### OR\n");
     if (p -> m_attribute.m_lattice_elem != TOP) {
         fprintf( m_outputfile, " pushl $%d\n", p -> m_attribute.m_lattice_elem.value);
         return;
     }

     p -> visit_children(this);

     fprintf( m_outputfile, " popl %%ebx\n");
     fprintf( m_outputfile, " popl %%eax\n");
     fprintf( m_outputfile, " orl %%ebx, %%eax\n");
     fprintf( m_outputfile, " pushl %%eax\n");
  }
  void visitMinus(Minus * p)
  {
   fprintf( m_outputfile, "#### MINUS\n");
//   if (p -> m_attribute.m_lattice_elem != TOP) {
//       fprintf( m_outputfile, " pushl $%d\n", p -> m_attribute.m_lattice_elem.value);
//       return;
//   }

   p -> visit_children(this);

    fprintf( m_outputfile, " popl %%ebx\n");
    fprintf( m_outputfile, " popl %%eax\n");
    fprintf( m_outputfile, " subl %%ebx, %%eax\n");
    fprintf( m_outputfile, " pushl %%eax\n");
  }
  void visitPlus(Plus * p)
  {
     fprintf( m_outputfile, "#### PLUS\n");
//     if (p -> m_attribute.m_lattice_elem != TOP) {
//         fprintf( m_outputfile, "pushl $%d\n", p -> m_attribute.m_lattice_elem.value);
//         return;
//     }

     p -> visit_children(this);

     fprintf( m_outputfile, "popl %%ebx\n");
     fprintf( m_outputfile, "popl %%eax\n");
     fprintf( m_outputfile, "addl %%ebx, %%eax\n");
     fprintf( m_outputfile, "pushl %%eax\n");

  }
  void visitTimes(Times * p)
  {
     fprintf( m_outputfile, "#### TIMES\n");
//     if (p -> m_attribute.m_lattice_elem != TOP) {
//         fprintf( m_outputfile, " pushl $%d\n", p -> m_attribute.m_lattice_elem.value);
//         return;
//     }

     p -> visit_children(this);

     fprintf( m_outputfile, " popl %%ebx\n");
     fprintf( m_outputfile, " popl %%eax\n");
     fprintf( m_outputfile, " imull %%ebx, %%eax\n");
     fprintf( m_outputfile, " pushl %%eax\n");
  }
  void visitDiv(Div * p)
  {
    fprintf( m_outputfile, "#### DIVIDE\n");
//    if (p -> m_attribute.m_lattice_elem != TOP) {
//         fprintf( m_outputfile, " pushl $%d\n", p -> m_attribute.m_lattice_elem.value);
//         return;
//    }

     p -> visit_children(this);

     fprintf( m_outputfile, " popl %%ebx\n");
     fprintf( m_outputfile, " popl %%eax\n");
     fprintf( m_outputfile, " cdq\n");
     fprintf( m_outputfile, " idivl %%ebx\n");
     fprintf( m_outputfile, " pushl %%eax\n");
  }
  void visitNot(Not * p)
  {
  fprintf( m_outputfile, "#### NOT\n");
//         if (p -> m_attribute.m_lattice_elem != TOP) {
//              fprintf( m_outputfile, " pushl $%d\n", p -> m_attribute.m_lattice_elem.value);
//              return;
//         }

          p -> visit_children(this);

          fprintf( m_outputfile, " popl %%eax\n");
          fprintf( m_outputfile, " not  %%eax\n");
          fprintf( m_outputfile, " pushl %%eax\n");
  }
  void visitUminus(Uminus * p)
  {
    fprintf( m_outputfile, "#### Uminus\n");
//       if (p -> m_attribute.m_lattice_elem != TOP) {
//            fprintf( m_outputfile, " pushl $%d\n", p -> m_attribute.m_lattice_elem.value);
//            return;
//       }

        p -> visit_children(this);

        fprintf( m_outputfile, " popl %%eax\n");
        fprintf( m_outputfile, " negl %%eax\n");
        fprintf( m_outputfile, " pushl %%eax\n");
  }
  void visitMagnitude(Magnitude * p)
  {
//    fprintf( m_outputfile, "#### Magnitude\n");
//           if (p -> m_attribute.m_lattice_elem != TOP) {
//                fprintf( m_outputfile, " pushl $%d\n", p -> m_attribute.m_lattice_elem.value);
//                return;
//           }

            p -> visit_children(this);

            fprintf( m_outputfile, " popl %%eax\n");
            fprintf( m_outputfile, "cdq\n");
            fprintf( m_outputfile, "xorl %%edx, %%eax\n");
            fprintf( m_outputfile, " subl %%edx, %%eax\n");
            fprintf( m_outputfile, " pushl %%eax\n");
  }

  // variable and constant access
  void visitIdent(Ident * p)
  {
   
    fprintf( m_outputfile, "#### Visit ID\n");
    p -> visit_children(this);
//    if (p->m_attribute.m_lattice_elem != TOP) {
//        fprintf( m_outputfile, "pushl $%d\n", p -> m_attribute.m_lattice_elem.value);
//        return;
//    }
    
    Symbol *s=m_st->lookup(p->m_attribute.m_scope,strdup(p->m_symname->spelling()));
    int offset=4+s->get_offset();
    fprintf( m_outputfile, "pushl -%d(%%ebp)\n",offset);
  }
  void visitIntLit(IntLit * p)
  {
    fprintf( m_outputfile, "#### Visit INT\n");
    fprintf( m_outputfile, "pushl $%d\n", p -> m_attribute.m_lattice_elem.value);

  }
  void visitBoolLit(BoolLit * p)
  {
    // WRITEME
          fprintf( m_outputfile, "#### Visit BOOLLit\n");
    fprintf( m_outputfile, "pushl $%d\n", p -> m_attribute.m_lattice_elem.value);

  }
  void visitArrayAccess(ArrayAccess * p)
  {
      fprintf( m_outputfile, "#### Visit ArrayAccess Children\n");
      p->m_expr->accept(this);
      fprintf( m_outputfile, "#### Visit ArrayAccess \n");
      Symbol *s=m_st->lookup(p->m_attribute.m_scope,strdup(p->m_symname->spelling()));
      int offset=s->get_offset()+4*p->m_expr->m_attribute.m_lattice_elem.value;
      fprintf( m_outputfile, "pop %%eax\n");
      
//      fprintf( m_outputfile, "mov -%d(ebp), %%ebx\n",offset);
      fprintf( m_outputfile, "movl %%eax, -%d(%%ebp)\n",offset);
      fprintf( m_outputfile, "pushl -%d(%%ebp)\n",offset);
  }

  // special cases
  void visitSymName(SymName * p)
  {
  }
  void visitPrimitive(Primitive * p)
  {
  }
};

