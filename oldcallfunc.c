				Object *function;
				Object *totalargs;
				Object *retval;
				como_function *fn;
				como_op_array *old_op_array;
				ssize_t i;

				como_frame *frame = como_frame_create();

				POP(function);
				OBJECT_DUMP(function);
				POP(totalargs);
				
				ENSURE_POINTER_TYPE(function);

				if(!(O_FLG(function) & COMO_IS_FUNCTION)) {
					como_error_noreturn("value is not callable");
				}

				fn = (como_function *)O_PTVAL(function);
				old_op_array = CG(op_array);

				como_debug("function %s has %zu args defined", O_SVAL(fn->name)->value,
					O_AVAL(fn->arguments)->size);

				i = (ssize_t)O_AVAL(fn->arguments)->size;

				if((long)i != O_LVAL(totalargs)) {
					como_error_noreturn("function `%s' expects exactly %zu arguments, %ld given",
						O_SVAL(fn->name)->value, O_AVAL(fn->arguments)->size, O_LVAL(totalargs));
				}


				while(i--) {
					Object *arg_value;
					POP(arg_value);
					Object *arg_name = O_AVAL(fn->arguments)->table[i];
					char *argsvalue = objectToString(arg_value);
					como_debug("%s: => %s", O_SVAL(arg_name)->value, argsvalue);
					free(argsvalue);
					mapInsert(fn->op_array->frame->cf_symtab, O_SVAL(arg_name)->value, 
						arg_value);
					fn->op_array->frame->cf_stack[fn->op_array->frame->cf_sp++] = arg_value;
				}


				fn->op_array->frame->cf_stack[fn->op_array->frame->cf_sp++] = totalargs;
				fn->op_array->frame->cf_stack[fn->op_array->frame->cf_sp++] = function;

				como_debug("calling function `%s' with %ld arguments", O_SVAL(fn->name)->value,
					O_LVAL(totalargs));

				como_stack_push(&CG(scope), (void *)fn->op_array->frame->cf_symtab);

				/* Set the executor to this functions op array */
				CG(op_array) = fn->op_array;



		

				//debug_code_to_output(stdout);
				//exit(1);

				(void )como_vm();

				if(CG(op_array)->frame->cf_retval != NULL) {
					como_debug("returning from function `%s'", O_SVAL(fn->name)->value);
					retval = copyObject(fn->op_array->frame->cf_retval);
				} else {
					como_debug("return value was null");
				}

				/* Reset the functions instruction pointer */
				CG(op_array)->pc = 0;
				CG(op_array)->frame->cf_sp = 0;

				fn->op_array->frame->cf_sp = 0;
				fn->op_array->frame->cf_retval = NULL;


				CG(op_array) = old_op_array;

				/* pop frame */
				CG(scope) = CG(scope)->next;

				//CG(op_array)->frame->cf_retval = retval;

				PUSH(retval);

				VM_CONTINUE