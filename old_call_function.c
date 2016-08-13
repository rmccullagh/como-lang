
			TARGET(CALL_FUNCTION) {
				Object *code = NULL;
				como_stack* top = eg->frame_stack;
				como_stack *prev = NULL;
				compiler_context *tmpctx = NULL;
				while(top != NULL) {
					tmpctx = (compiler_context *)top->value;
					code = mapSearchEx(tmpctx->function_table, O_SVAL(c->op1)->value);
					if(code != NULL) {
						break;
					}
					top = top->next;
					prev = top;
				}
				
				if(code == NULL) {
					como_error_noreturn("call to undefined function '%s'", O_SVAL(c->op1)->value);
				}

				Object *total_args;
				Object *colno;
				Object *lineno;

				POP(total_args);
				POP(colno);
				POP(lineno);

				como_function *fn;
				ssize_t i;

				fn = (como_function *)O_PTVAL(code);
				Array *arguments = O_AVAL(fn->arguments);

				if(fn->fn_arg_count != (size_t)O_LVAL(total_args)) {
					como_error_noreturn("function '%s' expects exactly %zu arguments, %ld given",
						O_SVAL(c->op1)->value, fn->fn_arg_count, O_LVAL(total_args));
				}

				i = (ssize_t)fn->fn_arg_count;
				while(i--) {
					Object *arg_value;
					POP(arg_value);
					Object *arg_name = arguments->table[i];
					/* It's important to copy the value of arg_value (mapInsert) here
					   as, if we don't, then arguments will actually be passed by reference, not value
					 */
					mapInsert(fn->fn_frame->cf_symtab, O_SVAL(arg_name)->value, 
						arg_value);
				}

				/* save context */
				compiler_context *old_ctx = cg;
				como_frame *_old_cframe = cframe;

				como_debug("calling function '%s' with %ld arguments", O_SVAL(c->op1)->value,
					O_LVAL(total_args));



				cg = fn->fn_ctx;
				cframe = fn->fn_frame;


				if(cg->code.table[cg->code.size - 1]->inst.opcode != IRETURN) {
					como_debug("automatically inserting IRETURN for function %s", O_SVAL(c->op1)->value);
					emit(LOAD_CONST, newLong(0L));
					emit(IRETURN, newLong(1L));
				}

				call_stack_push(c->op1, lineno, colno);
				/*
				 * push this frame onto the frame stack
				 */
				frame_stack_push(fn->fn_ctx);

				//return;
				(void)como_vm();

				Object *retval = copyObject(cframe->cf_retval);

				como_debug("stack pointer is at %zu", cframe->cf_sp);
				
				/* restore context */
				cg = old_ctx;
				cframe = _old_cframe;
				
				/* BEGIN RESET */
				
				fn->fn_frame->cf_retval = NULL;
				objectDestroy(fn->fn_frame->cf_symtab);
				fn->fn_frame->cf_symtab = newMap(2);
				fn->fn_frame->cf_sp = 0;
				fn->fn_ctx->pc = 0;
				size_t ii;
				for(ii = 0; ii < CF_STACKSIZE; ii++) {
					fn->fn_frame->cf_stack[ii] = NULL;
				}
				
				/* END RESET */

				/*
				 * TODO pop the frame_stack and kill all objects in it 
				 */
				 
				if(prev) {
					como_debug("popping frame stack");
					//prev->next = top->next;
				} else {
					//eg->frame_stack = top->next;
				}
			
				PUSH(retval);
				VM_CONTINUE
			}