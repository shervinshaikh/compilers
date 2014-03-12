#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "assert.h"
#include <typeinfo>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#define WORDSIZE 4
#define OPTIMIZE 1

class Codegen : public Visitor
{
    private:

        FILE * m_outputfile;
        SymTab *m_st;

        // basic size of a word (integers and booleans) in bytes
        static const int wordsize = 1;

        int label_count; //access with new_label

        // ********** Helper functions ********************************

        // this is used to get new unique labels (cleverly named label1, label2, ...)
        int new_label() { return label_count++; }

        // this mode is used for the code
        void set_text_mode() { fprintf( m_outputfile, ".text\n\n"); }

        // PART 1:
        // 1) get arithmetic expressions on integers working:
        //    you wont really be able to run your code,
        //    but you can visually inspect it to see that the correct
        //<F2><F2><F2>    chains of opcodes are being generated.
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
        //        ||    
        //        ||
        //       \  /
        //        \/
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
        //    return address of the stack. After the ret, remove the arguments from the stack
        //
        //  For more info on this convention, see http://unixwiz.net/techtips/win32-callconv-asm.html
        //
        //  This convention is called __cdecl
        //
        //////////////////////////////////////////////////////////////////////////////

        void emit_prologue(SymName *name, unsigned int size_locals, unsigned int num_args)
        {
            //int total_size = (size_locals +num_args+1)*WORDSIZE;
            int total_size = size_locals;
            stringstream ss;
            ss
                <<name->spelling()<<":"<<endl
                <<"\t"<<"pushl \%ebp"<<" #Save Original EBP"<<endl
                <<"\t"<<"movl \%esp,\%ebp"<<" #Point EBP to top of stack"<<endl
                <<"\t"<<"subl $"<<total_size<<",\%esp"<<" #make room for local variables"<<endl;

            int param_off=0;
            int local_off=0;
            for (int i =0; i< num_args;i++)
            {
#if 1
                param_off = (2+i)*WORDSIZE;
                local_off = -(4*i+4); 
                ss
                
                    <<"\t"<<"movl "<<param_off<<"(\%ebp),\%ebx"<<endl
                    <<"\t"<<"movl \%ebx,"<<local_off<<"(\%ebp)"<<endl;
#endif 

            }
            ss
                <<"\t"<<"pushl \%ebx#Save Used Registor"<<endl;
            fprintf( m_outputfile, "%s", ss.str().c_str());

        }
        bool emit_optimize(LatticeElem e){
#if OPTIMIZE
            stringstream ss;
            if(e!=TOP){
                ss<<"\t"<<"pushl $"<<e.value<<"#OPT:"<<e.value<<endl;
                fprintf( m_outputfile, "%s", ss.str().c_str());
                return true;
            }
            if(e==BOTTOM){
                fprintf( m_outputfile, "#SOMETHING IS BOTTOM O GAWD\n", ss.str().c_str());
            }
#endif
            return false;

        }
        void emit_logic(string op){
            stringstream ss;
            ss
                <<"\t"<<"popl \%ebx"<<"#Starting:"<<op<<endl
                <<"\t"<<"popl \%eax"<<endl
                <<"\t"<<"cmpl \%ebx,\%eax"<<endl
                <<"\t"<<op<<" \%al"<<endl
                <<"\t"<<"movzbl %al,%eax"<<endl
                <<"\t"<<"pushl \%eax"<<endl;
            fprintf( m_outputfile, "%s", ss.str().c_str());
        }

        void emit_arith(string op){
            stringstream ss;
            string div = "idivl";
            if(op==div){
                ss
                    <<"\t"<<"popl \%ebx"<<endl
                    <<"\t"<<"popl \%eax"<<endl
                    <<"\t"<<"mov \%eax,\%edx"<<endl
                    <<"\t"<<"sar "<<"$0x1f,\%edx"<<endl
                    <<"\t"<<op<< " \%ebx"<<endl
                    <<"\t"<<"pushl \%eax"<<endl;


            }
            else{
                ss
                    <<"\t"<<"popl \%ebx "<<endl
                    <<"\t"<<"popl \%eax"<<endl
                    <<"\t"<<op<< " \%ebx,\%eax"<<endl
                    <<"\t"<<"pushl \%eax"<<endl;
            }
            fprintf( m_outputfile, "%s", ss.str().c_str());


        }

        void emit_epilogue()
        {
            stringstream ss;
            ss
                <<"\t"<<"popl \%ebx#save store register"<<endl
                <<"\t"<<"movl \%ebp,\%esp"<<endl
                <<"\t"<<"popl \%ebp"<<endl
                <<"\t"<<"retl"<<endl;
            fprintf( m_outputfile, "%s", ss.str().c_str());
        }

        // HERE: more functions to emit code

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
            stringstream ss;
            ss
                <<".globl Main"<<endl;
            fprintf( m_outputfile, "%s", ss.str().c_str());
            visit_children_of(p);


        }
        void visitFunc(Func * p)
        {
            int size_locals = m_st->scopesize(p->m_function_block->m_attribute.m_scope) +4;
            emit_prologue(p->m_symname, size_locals,
                    p->m_param_list->size());
            visit_children_of(p);


        }
        void visitFunction_block(Function_block * p)
        {

            stringstream ss;
#if 1
            int i = 0;
            list<Decl_ptr>::iterator m_decl_list_iter;
            list<SymName_ptr>::iterator m_symname_list_iter;
            for(m_decl_list_iter = p->m_decl_list->begin();
                    m_decl_list_iter != p->m_decl_list->end();
                    ++m_decl_list_iter){
                /*
                   for(m_symname_list_iter= (*m_decl_list_iter)->m_symname_list->begin();
                   m_symname_list_iter != (*m_decl_list_iter)->m_symname_list->end(); ++m_symname_list_iter){
                   Symbol *s = new Symbol();

                   char * name= strdup((*m_symname_list_iter)->spelling());
                   m_st -> insert(name, s);
                 */
                // add_decl_symbol((*m_decl_list_iter));

                fprintf( m_outputfile, "%s", ss.str().c_str());

            }

#endif
            visit_children_of(p);


            }
            void visitNested_block(Nested_block * p)
            {
                
                stringstream ss;
                p->visit_children(this);
            }
            void visitAssignment(Assignment * p)
            {
                stringstream ss;

                p->visit_children(this);
#if 1
                const char * name = p->m_symname->spelling();
                int off= m_st->lookup( p->m_attribute.m_scope,name)->get_offset();
                off = -(off+4);
                p->m_symname->m_attribute.m_place = off;
                ss
                    <<"\t"<<"popl \%ebx"<<endl
                    <<"\t"<<"movl \%ebx,"<<off<<"(\%ebp)"<<endl;

                fprintf( m_outputfile, "%s", ss.str().c_str());
#endif
            }
            void visitArrayAssignment(ArrayAssignment * p)
            {
                stringstream ss;

                visit(p->m_symname);
                const char * name = p->m_symname->spelling();
                int off= m_st->lookup( p->m_attribute.m_scope,name)->get_offset();
                off = -(off+4);
                p->m_symname->m_attribute.m_place = off;
                visit(p->m_expr_1);


                ss
                    <<"\t"<<"pushl $4"<<endl;
                fprintf( m_outputfile, "%s", ss.str().c_str());
                ss.str("");
                emit_arith("imull");
                visit(p->m_expr_2);
                ss
                    <<"\t"<<"popl \%eax"<<endl
                    <<"\t"<<"popl \%ebx"<<endl
                    <<"\t"<<"movl \%eax,"<<off<<"(\%ebp, \%ebx,4)"<<endl;

                fprintf( m_outputfile, "%s", ss.str().c_str());


            }
            void visitCall(Call * p)
            {

                visit(p->m_symname_1);
                visit(p->m_symname_2);
                stringstream ss;
                list<Expr_ptr>::iterator m_expr_list_iter;
                int param_size = p->m_expr_list->size();
                int i =1;
#if 1

                // for(m_expr_list_iter = p->m_expr_list->end() - 1;
                //         m_expr_list_iter != p->m_expr_list->begin();
                //         --m_expr_list_iter){
                list<Expr_ptr> * ab  =  new list<Expr_ptr>(*p->m_expr_list);
                for(int i=0;i<param_size;i++){

                    visit(ab->back());
                    ab->pop_back();

                    //      (*m_expr_list_iter)->m_attribute.m_place = (param_size-i-1)*WORDSIZE;
                }
#endif


#if 0
                int param_size = p->m_expr_list->size();
                for (int i=0;i<param_size;i++)
                {
                    ss
                        <<"\t"<<"popl \%ebx"<<"#start visit call"<<endl
                        <<"\t"<<"movl \%ebx,"<<(param_size-i-1)*WORDSIZE<<"(\%esp)"
                        <<endl;
                    *(p->m_expr_list[i])m_place=(param_size-i-1)*WORDSIZE;

                }

#endif
                ss
                    <<"\t"<<"call "<< p->m_symname_2->spelling()<<endl;
#if 1
                int off = -(m_st->lookup(p->m_attribute.m_scope,p->m_symname_1->spelling())->get_offset())-4; 
                ss
                    <<"\tmovl "<<"\%eax,"<<off<<"(\%ebp)"<<"#end visit call"<<endl;
                for(m_expr_list_iter = p->m_expr_list->begin();
                        m_expr_list_iter != p->m_expr_list->end();
                        ++m_expr_list_iter){
                    ss
                        <<"\t"<<"popl \%ebx"<<endl;
                }
                //   cout<<p->m_symname->spelling()<<endl;

#endif





                fprintf( m_outputfile, "%s", ss.str().c_str());

                // WRITEME
        // p->visit_children(this);
            }
            void visitArrayCall(ArrayCall *p)
            {
                visit(p->m_symname_1);
                visit(p->m_symname_2);
                stringstream ss;
                list<Expr_ptr>::iterator m_expr_list_iter;
                int param_size = p->m_expr_list_2->size();
                int i =1;
#if 1

                // for(m_expr_list_iter = p->m_expr_list->end() - 1;
                //         m_expr_list_iter != p->m_expr_list->begin();
                //         --m_expr_list_iter){
                list<Expr_ptr> * ab  =  new list<Expr_ptr>(*p->m_expr_list_2);
                for(int i=0;i<param_size;i++){

                    visit(ab->back());
                    ab->pop_back();

                    ss
                        <<"\t"<<"popl \%ebx"<<"#start visit call"<<endl
                        //<<"\t"<<"movl \%ebx,"<<(param_size-i-1)*WORDSIZE<<"(\%esp)"
                        <<"\t"<<"pushl \%ebx"<<endl;
                    //      (*m_expr_list_iter)->m_attribute.m_place = (param_size-i-1)*WORDSIZE;
                }
#endif


#if 0
                int param_size = p->m_expr_list->size();
                for (int i=0;i<param_size;i++)
                {
                    ss
                        <<"\t"<<"popl \%ebx"<<"#start visit call"<<endl
                        <<"\t"<<"movl \%ebx,"<<(param_size-i-1)*WORDSIZE<<"(\%esp)"
                        <<endl;
                    *(p->m_expr_list[i])m_place=(param_size-i-1)*WORDSIZE;

                }

#endif

                const char * name = p->m_symname_2->spelling();
                ss
                    <<"\t"<<"call "<< p->m_symname_2->spelling()<<endl;
                int off= m_st->lookup( p->m_attribute.m_scope,name)->get_offset();
                off = -(off+4);
                p->m_symname_2->m_attribute.m_place = off;


                
#if 1                
                for(m_expr_list_iter = p->m_expr_list_2->begin();
                        m_expr_list_iter != p->m_expr_list_2->end();
                        ++m_expr_list_iter){
                    ss
                        <<"\t"<<"popl \%ebx"<<endl;
                }
                    ss
                        <<"\t"<<"push \%eax"<<endl;

                fprintf( m_outputfile, "%s", ss.str().c_str());
                ss.str("");
                visit(p->m_expr_1);

                ss
                    <<"\t"<<"pushl $4"<<endl;
                fprintf( m_outputfile, "%s", ss.str().c_str());
                ss.str("");
                emit_arith("imull");
                ss
                    <<"\t"<<"popl \%eax"<<endl
                    <<"\t"<<"popl \%ebx"<<endl
                    <<"\t"<<"movl \%ebx,"<<off<<"(\%ebp, \%eax,4)"<<endl;




#endif





                fprintf( m_outputfile, "%s", ss.str().c_str());

            }
            void visitReturn(Return * p)
            {
                visit_children_of(p);
                stringstream ss;
                ss
                    <<"\t"<<"popl "<<"\%eax"<<endl;
                fprintf( m_outputfile, "%s", ss.str().c_str());
                emit_epilogue();

            }

            // control flow
            void visitIfNoElse(IfNoElse * p)
            {
                stringstream ss;
                
                if (p->m_expr->m_attribute.m_lattice_elem == TOP || !OPTIMIZE){
                    visit(p->m_expr);
                    int bottom = new_label();
                    ss
                        <<"\t"<<"popl \%eax#start IfwithNoElse"<<endl
                        <<"\t"<<"cmpl $1,\%eax"<<endl
                        <<"\t"<<"jne label"<<bottom<<endl;
                    fprintf( m_outputfile, "%s", ss.str().c_str());
                    visit(p->m_nested_block);
                    ss.str("");
                    ss
                        <<"label"<<bottom<<":#end if"<<endl;
                    fprintf( m_outputfile, "%s", ss.str().c_str());
                }
                else if(p->m_expr->m_attribute.m_lattice_elem.value==1)
                {
                    visit(p->m_nested_block);
                    
                }




            }
            void visitIfWithElse(IfWithElse * p)
            {

                visit(p->m_expr);
            if (p->m_expr->m_attribute.m_lattice_elem == TOP || !OPTIMIZE){
                int block1_lb = new_label();
                int block2_lb = new_label();
                stringstream ss;
                ss
                    <<"\t"<<"popl \%eax#start IfwithElse"<<endl
                    <<"\t"<<"cmpl $1,\%eax"<<endl
                    <<"\t"<<"jne label"<<block1_lb<<endl;
                fprintf( m_outputfile, "%s", ss.str().c_str());
                visit(p->m_nested_block_1);
                ss.str("");
                ss
                    <<"\t"<<"jmp label"<<block2_lb<<endl
                    <<"label"<<block1_lb<<":#end if"<<endl;
                fprintf( m_outputfile, "%s", ss.str().c_str());
                ss.str("");
                visit(p->m_nested_block_2);
                ss.str("");
                ss
                    <<"label"<<block2_lb<<":#end if"<<endl;
                fprintf( m_outputfile, "%s", ss.str().c_str());
            }
            else if(p->m_expr->m_attribute.m_lattice_elem.value==1){

                    visit(p->m_nested_block_1);
            }
            else{

                    visit(p->m_nested_block_2);
                
            }




            }
            void visitForLoop(ForLoop * p)
            {
                stringstream ss;
            if (p->m_expr->m_attribute.m_lattice_elem == TOP || !OPTIMIZE){
                visit(p->m_stat_1);

                int top = new_label();
                int bottom = new_label();
                ss
                    <<"label"<<top<<":#top"<<endl;
                fprintf( m_outputfile, "%s", ss.str().c_str());

                ss.str("");
                visit(p->m_expr);

                ss
                    <<"\t"<<"popl \%eax#start IfwithNoElse"<<endl
                    <<"\t"<<"cmpl $1,\%eax"<<endl
                    <<"\t"<<"jne label"<<bottom<<endl;
                fprintf( m_outputfile, "%s", ss.str().c_str());
                visit(p->m_nested_block);
                visit(p->m_stat_2);
                ss.str("");
                ss
                    <<"\t"<<"jmp label"<<top<<endl
                    <<"label"<<bottom<<":#end if"<<endl;
                fprintf( m_outputfile, "%s", ss.str().c_str());
            }





            }


            void visitNone(None *p)
            {
                // Nothing is emitted
            }

            // variable declarations (no code generation needed)
            void visitDecl(Decl * p)
            {
            }
            void visitParam(Param *p)
            {
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
                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    emit_logic("sete");
                }
            }
            void visitNoteq(Noteq * p)
            {
                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    emit_logic("setne");
                }
            }
            void visitGt(Gt * p)
            {
                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    emit_logic("setg");
                }
            }
            void visitGteq(Gteq * p)
            {
                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    emit_logic("setge");
                }
            }
            void visitLt(Lt * p)
            {
                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    emit_logic("setl");
                }
            }
            void visitLteq(Lteq * p)
            {
                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    emit_logic("setle");
                }
            }

            // arithmetic and logic operations
            void visitAnd(And * p)
            {


                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    emit_arith("andl");
                }
            }
            void visitOr(Or * p)
            {
                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    emit_arith("orl");
                }
            }
            void visitMinus(Minus * p)
            {
                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    emit_arith("subl");
                }
            }
            void visitPlus(Plus * p)
            {
                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    emit_arith("addl");
                }
            }
            void visitTimes(Times * p)
            {
                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    emit_arith("imull");
                }
            }
            void visitDiv(Div * p)
            {

                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    emit_arith("idivl");
                }
            }
            void visitNot(Not * p)
            {
                stringstream ss;

                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    ss
                        <<"pushl $0"<<endl;
                    fprintf( m_outputfile, "%s", ss.str().c_str());
                    emit_arith("xorl");
                }

            }
            void visitUminus(Uminus * p)
            {
                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    stringstream ss;
                    ss<<"pushl $-1"<<endl;
                    fprintf( m_outputfile, "%s", ss.str().c_str());
                    emit_arith("imull");
                }
            }
            void visitMagnitude(Magnitude * p)
            {
                if (!emit_optimize(p->m_attribute.m_lattice_elem)){
                    p->visit_children(this);
                    stringstream ss;
                    ss
                        <<"\t"<<"popl \%ebx"<<endl
                        <<"\t"<<"cdq"<<endl
                        <<"\t"<<"xor \%ebx, \%eax"<<endl
                        <<"\t"<<"sub \%ebx, \%eax"<<endl
                        <<"\t"<<"pushl \%eax" <<endl;
                    fprintf( m_outputfile, "%s", ss.str().c_str());
                }
            }

            // variable and constant access
            void visitIdent(Ident * p)
            {
                stringstream ss;
                p->visit_children(this);
#if 1
                //   cout<<p->m_symname->spelling()<<endl;
                int off = -(m_st->lookup( p->m_attribute.m_scope,p->m_symname->spelling())->get_offset())-4;
                ss
                    <<"\tmovl "<<off<<"(\%ebp),"<<"\%ebx"<<endl
                    <<"\t"<<"pushl \%ebx"<<endl;

                fprintf( m_outputfile, "%s", ss.str().c_str());
#endif

                // WRITEME
            }
            void visitIntLit(IntLit * p)
            {
                p->visit_children(this);
                stringstream ss;
                ss  <<"\t"<<"pushl $"<<p->m_primitive->m_data<<endl;
                fprintf( m_outputfile, "%s", ss.str().c_str());
            }
            void visitBoolLit(BoolLit * p)
            {
                p->visit_children(this);
                stringstream ss;
                ss  <<"\t"<<"pushl $"<<p->m_primitive->m_data<<endl;
                fprintf( m_outputfile, "%s", ss.str().c_str());
                // WRITEME
            }
            void visitArrayAccess(ArrayAccess * p)
            {
                stringstream ss;
                p->visit_children(this);
#if 1
                //   cout<<p->m_symname->spelling()<<endl;
                int off = -(m_st->lookup( p->m_attribute.m_scope,p->m_symname->spelling())->get_offset())-4;
                ss<<"pushl $4"<<endl;
                fprintf( m_outputfile, "%s", ss.str().c_str());
                ss.str("");
                emit_arith("imull");
                ss
                    <<"\tpopl \%eax "<<endl
                    <<"\tmovl "<<off<<"(\%ebp, \%eax, 4),"<<"\%ebx"<<endl
                    <<"\t"<<"pushl \%ebx"<<endl;

                fprintf( m_outputfile, "%s", ss.str().c_str());
#endif


            }

            // special cases
            void visitSymName(SymName * p)
            {
            }
            void visitPrimitive(Primitive * p)
            {
            }
            };
